// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2003-2015 The Boeing Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef UIRESOURCES_HPP
#define UIRESOURCES_HPP

#include <QFileIconProvider>
#include <QFont>
#include <QIcon>
#include <QImage>
#include <QMap>

#include "ViExport.hpp"

namespace wizard
{
//! This class contains the loaded icons used by the application.
class VI_EXPORT UiResources
{
public:
   static constexpr unsigned int cBASE_ICON_SIZE = 16;
   static constexpr unsigned int cMIN_FONT_SIZE  = 6;
   static constexpr unsigned int cMAX_FONT_SIZE  = 24;

   //! Virtual destructor
   virtual ~UiResources();

   //! Singleton instance
   static void         Create();
   static void         Destroy();
   static UiResources& GetInstance() { return *sInstancePtr; }

   // Fonts
   QFont& GetFont() { return mConsoleFont; }

   // Icons
   QIcon& GetIcon(const QString& aImageName, bool aScale = true);
   QIcon& GetIconFromTheme(const QString& aImageName,
                           bool           aScale = true); // Use this instead of QIcon::fromTheme in model views
   QIcon  GetFileIcon(const QFileInfo& aInfo);

   // Images
   QImage& GetImageFromTheme(const QString& aImageName, bool aScale = true);
   QImage& GetDimImageFromTheme(const QString& aImageName, bool aScale = true);

   //! Will vary image size based on font size between current limits of 24U and 6U found in wizard::EditorManager::AdjustFontSize
   std::pair<unsigned int, unsigned int> ResizeImageWithFont();

protected:
   void Initialize();

   QImage  ToScale(QImage aImage, int aWidth);
   QPixmap ToScalePM(QImage aImage, int aWidth);

   // Icons
   QIcon& AddIcon(const QString& aIconName, bool aScale);

   // Images
   QImage& AddImage(const QString& aImageName, bool aScale);
   QImage& AddDimImage(const QString& aImageName, bool aScale);

private:
   // Fonts
   QFont mConsoleFont;

   // Icons
   QMap<QString, QIcon> mIcons;
   QIcon                mEmptyIcon;
   QFileIconProvider    mIconProvider;

   QMap<QString, QImage> mImages;
   QMap<QString, QImage> mDimImages;
   QImage                mEmptyImage;

   static UiResources* sInstancePtr;
};
} // namespace wizard
#endif
