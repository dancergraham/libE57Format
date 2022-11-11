/*
 * Original work Copyright 2009 - 2010 Kevin Ackley (kackley@gwi.net)
 * Modified work Copyright 2018 - 2020 Andy Maloney <asmaloney@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "CompressedVectorNodeImpl.h"
#include "CheckedFile.h"
#include "CompressedVectorReaderImpl.h"
#include "CompressedVectorWriterImpl.h"
#include "ImageFileImpl.h"
#include "StringFunctions.h"
#include "VectorNodeImpl.h"

namespace e57
{
   CompressedVectorNodeImpl::CompressedVectorNodeImpl( ImageFileImplWeakPtr destImageFile ) : NodeImpl( destImageFile )
   {
      // don't checkImageFileOpen, NodeImpl() will do it
   }

   void CompressedVectorNodeImpl::setPrototype( const NodeImplSharedPtr &prototype )
   {
      // don't checkImageFileOpen, ctor did it

      //??? check ok for proto, no Blob CompressedVector, empty?
      //??? throw E57_EXCEPTION2(ErrorBadPrototype)

      /// Can't set prototype twice.
      if ( prototype_ )
      {
         throw E57_EXCEPTION2( ErrorSetTwice, "this->pathName=" + this->pathName() );
      }

      /// prototype can't have a parent (must be a root node)
      if ( !prototype->isRoot() )
      {
         throw E57_EXCEPTION2( ErrorAlreadyHasParent,
                               "this->pathName=" + this->pathName() + " prototype->pathName=" + prototype->pathName() );
      }

      /// Verify that prototype is destined for same ImageFile as this is
      ImageFileImplSharedPtr thisDest( destImageFile() );
      ImageFileImplSharedPtr prototypeDest( prototype->destImageFile() );
      if ( thisDest != prototypeDest )
      {
         throw E57_EXCEPTION2( ErrorDifferentDestImageFile, "this->destImageFile" + thisDest->fileName() +
                                                               " prototype->destImageFile" +
                                                               prototypeDest->fileName() );
      }

      //!!! check for incomplete CompressedVectors when closing file
      prototype_ = prototype;

      /// Note that prototype is not attached to CompressedVector in a parent/child
      /// relationship. This means that prototype is a root node (has no parent).
   }

   NodeImplSharedPtr CompressedVectorNodeImpl::getPrototype() const
   {
      checkImageFileOpen( __FILE__, __LINE__, static_cast<const char *>( __FUNCTION__ ) );
      return ( prototype_ ); //??? check defined
   }

   void CompressedVectorNodeImpl::setCodecs( const std::shared_ptr<VectorNodeImpl> &codecs )
   {
      // don't checkImageFileOpen, ctor did it

      //??? check ok for codecs, empty vector, or each element has "inputs" vector
      // of strings, codec
      // substruct

      /// Can't set codecs twice.
      if ( codecs_ )
      {
         throw E57_EXCEPTION2( ErrorSetTwice, "this->pathName=" + this->pathName() );
      }

      /// codecs can't have a parent (must be a root node)
      if ( !codecs->isRoot() )
      {
         throw E57_EXCEPTION2( ErrorAlreadyHasParent,
                               "this->pathName=" + this->pathName() + " codecs->pathName=" + codecs->pathName() );
      }

      /// Verify that codecs is destined for same ImageFile as this is
      ImageFileImplSharedPtr thisDest( destImageFile() );
      ImageFileImplSharedPtr codecsDest( codecs->destImageFile() );
      if ( thisDest != codecsDest )
      {
         throw E57_EXCEPTION2( ErrorDifferentDestImageFile, "this->destImageFile" + thisDest->fileName() +
                                                               " codecs->destImageFile" + codecsDest->fileName() );
      }

      codecs_ = codecs;

      /// Note that codecs is not attached to CompressedVector in a parent/child
      /// relationship. This means that codecs is a root node (has no parent).
   }

   std::shared_ptr<VectorNodeImpl> CompressedVectorNodeImpl::getCodecs() const
   {
      checkImageFileOpen( __FILE__, __LINE__, static_cast<const char *>( __FUNCTION__ ) );
      return ( codecs_ ); //??? check defined
   }

   bool CompressedVectorNodeImpl::isTypeEquivalent( NodeImplSharedPtr ni )
   {
      // don't checkImageFileOpen

      //??? is this test a good idea?

      /// Same node type?
      if ( ni->type() != TypeCompressedVector )
      {
         return ( false );
      }

      std::shared_ptr<CompressedVectorNodeImpl> cvi( std::static_pointer_cast<CompressedVectorNodeImpl>( ni ) );

      /// recordCount must match
      if ( recordCount_ != cvi->recordCount_ )
      {
         return ( false );
      }

      /// Prototypes and codecs must match ???
      if ( !prototype_->isTypeEquivalent( cvi->prototype_ ) )
      {
         return ( false );
      }
      if ( !codecs_->isTypeEquivalent( cvi->codecs_ ) )
      {
         return ( false );
      }

      return ( true );
   }

   bool CompressedVectorNodeImpl::isDefined( const ustring &pathName )
   {
      throw E57_EXCEPTION2( ErrorNotImplemented, "this->pathName=" + this->pathName() + " pathName=" + pathName );
   }

   void CompressedVectorNodeImpl::setAttachedRecursive()
   {
      /// Mark this node as attached to an ImageFile
      isAttached_ = true;

      /// Mark nodes in prototype tree, if defined
      if ( prototype_ )
      {
         prototype_->setAttachedRecursive();
      }

      /// Mark nodes in codecs tree if defined
      if ( codecs_ )
      {
         codecs_->setAttachedRecursive();
      }
   }

   int64_t CompressedVectorNodeImpl::childCount() const
   {
      checkImageFileOpen( __FILE__, __LINE__, static_cast<const char *>( __FUNCTION__ ) );
      return ( recordCount_ );
   }

   void CompressedVectorNodeImpl::checkLeavesInSet( const StringSet & /*pathNames*/, NodeImplSharedPtr /*origin*/ )
   {
      // don't checkImageFileOpen

      /// Since only called for prototype nodes, shouldn't be able to get here since
      /// CompressedVectors can't be in prototypes
      throw E57_EXCEPTION2( ErrorInternal, "this->pathName=" + this->pathName() );
   }

   void CompressedVectorNodeImpl::writeXml( ImageFileImplSharedPtr imf, CheckedFile &cf, int indent,
                                            const char *forcedFieldName )
   {
      // don't checkImageFileOpen

      ustring fieldName;
      if ( forcedFieldName != nullptr )
      {
         fieldName = forcedFieldName;
      }
      else
      {
         fieldName = elementName_;
      }

      uint64_t physicalStart = cf.logicalToPhysical( binarySectionLogicalStart_ );

      cf << space( indent ) << "<" << fieldName << " type=\"CompressedVector\"";
      cf << " fileOffset=\"" << physicalStart;
      cf << "\" recordCount=\"" << recordCount_ << "\">\n";

      if ( prototype_ )
      {
         prototype_->writeXml( imf, cf, indent + 2, "prototype" );
      }
      if ( codecs_ )
      {
         codecs_->writeXml( imf, cf, indent + 2, "codecs" );
      }
      cf << space( indent ) << "</" << fieldName << ">\n";
   }

#ifdef E57_DEBUG
   void CompressedVectorNodeImpl::dump( int indent, std::ostream &os ) const
   {
      os << space( indent ) << "type:        CompressedVector"
         << " (" << type() << ")" << std::endl;
      NodeImpl::dump( indent, os );
      if ( prototype_ )
      {
         os << space( indent ) << "prototype:" << std::endl;
         prototype_->dump( indent + 2, os );
      }
      else
      {
         os << space( indent ) << "prototype: <empty>" << std::endl;
      }
      if ( codecs_ )
      {
         os << space( indent ) << "codecs:" << std::endl;
         codecs_->dump( indent + 2, os );
      }
      else
      {
         os << space( indent ) << "codecs: <empty>" << std::endl;
      }
      os << space( indent ) << "recordCount:                " << recordCount_ << std::endl;
      os << space( indent ) << "binarySectionLogicalStart:  " << binarySectionLogicalStart_ << std::endl;
   }
#endif

   std::shared_ptr<CompressedVectorWriterImpl> CompressedVectorNodeImpl::writer( std::vector<SourceDestBuffer> sbufs )
   {
      checkImageFileOpen( __FILE__, __LINE__, static_cast<const char *>( __FUNCTION__ ) );

      ImageFileImplSharedPtr destImageFile( destImageFile_ );

      /// Check don't have any writers/readers open for this ImageFile
      if ( destImageFile->writerCount() > 0 )
      {
         throw E57_EXCEPTION2( ErrorTooManyWriters, "fileName=" + destImageFile->fileName() +
                                                       " writerCount=" + toString( destImageFile->writerCount() ) +
                                                       " readerCount=" + toString( destImageFile->readerCount() ) );
      }
      if ( destImageFile->readerCount() > 0 )
      {
         throw E57_EXCEPTION2( ErrorTooManyReaders, "fileName=" + destImageFile->fileName() +
                                                       " writerCount=" + toString( destImageFile->writerCount() ) +
                                                       " readerCount=" + toString( destImageFile->readerCount() ) );
      }

      /// sbufs can't be empty
      if ( sbufs.empty() )
      {
         throw E57_EXCEPTION2( ErrorBadAPIArgument, "fileName=" + destImageFile->fileName() );
      }

      if ( !destImageFile->isWriter() )
      {
         throw E57_EXCEPTION2( ErrorFileReadOnly, "fileName=" + destImageFile->fileName() );
      }

      if ( !isAttached() )
      {
         throw E57_EXCEPTION2( ErrorNodeUnattached, "fileName=" + destImageFile->fileName() );
      }

      /// Get pointer to me (really shared_ptr<CompressedVectorNodeImpl>)
      NodeImplSharedPtr ni( shared_from_this() );

      /// Downcast pointer to right type
      std::shared_ptr<CompressedVectorNodeImpl> cai( std::static_pointer_cast<CompressedVectorNodeImpl>( ni ) );

      /// Return a shared_ptr to new object
      std::shared_ptr<CompressedVectorWriterImpl> cvwi( new CompressedVectorWriterImpl( cai, sbufs ) );
      return ( cvwi );
   }

   std::shared_ptr<CompressedVectorReaderImpl> CompressedVectorNodeImpl::reader( std::vector<SourceDestBuffer> dbufs )
   {
      checkImageFileOpen( __FILE__, __LINE__, static_cast<const char *>( __FUNCTION__ ) );

      ImageFileImplSharedPtr destImageFile( destImageFile_ );

      /// Check don't have any writers/readers open for this ImageFile
      if ( destImageFile->writerCount() > 0 )
      {
         throw E57_EXCEPTION2( ErrorTooManyWriters, "fileName=" + destImageFile->fileName() +
                                                       " writerCount=" + toString( destImageFile->writerCount() ) +
                                                       " readerCount=" + toString( destImageFile->readerCount() ) );
      }
      if ( destImageFile->readerCount() > 0 )
      {
         throw E57_EXCEPTION2( ErrorTooManyReaders, "fileName=" + destImageFile->fileName() +
                                                       " writerCount=" + toString( destImageFile->writerCount() ) +
                                                       " readerCount=" + toString( destImageFile->readerCount() ) );
      }

      /// dbufs can't be empty
      if ( dbufs.empty() )
      {
         throw E57_EXCEPTION2( ErrorBadAPIArgument, "fileName=" + destImageFile->fileName() );
      }

      /// Can be read or write mode, but must be attached
      if ( !isAttached() )
      {
         throw E57_EXCEPTION2( ErrorNodeUnattached, "fileName=" + destImageFile->fileName() );
      }

      /// Get pointer to me (really shared_ptr<CompressedVectorNodeImpl>)
      NodeImplSharedPtr ni( shared_from_this() );
#ifdef E57_MAX_VERBOSE
      // cout << "constructing CAReader, ni:" << std::endl;
      // ni->dump(4);
#endif

      /// Downcast pointer to right type
      std::shared_ptr<CompressedVectorNodeImpl> cai( std::static_pointer_cast<CompressedVectorNodeImpl>( ni ) );
#ifdef E57_MAX_VERBOSE
      // cout<<"constructing CAReader, cai:"<<endl;
      // cai->dump(4);
#endif
      /// Return a shared_ptr to new object
      std::shared_ptr<CompressedVectorReaderImpl> cvri( new CompressedVectorReaderImpl( cai, dbufs ) );
      return ( cvri );
   }
}
