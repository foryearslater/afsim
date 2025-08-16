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

#include "UiResources.hpp"

#include <assert.h>

#include "EditorPrefObject.hpp"
#include "WkfEnvironment.hpp"

wizard::UiResources* wizard::UiResources::sInstancePtr = nullptr;

constexpr unsigned int wizard::UiResources::cBASE_ICON_SIZE;
constexpr unsigned int wizard::UiResources::cMIN_FONT_SIZE;
constexpr unsigned int wizard::UiResources::cMAX_FONT_SIZE;

// ============================================================================
// Virtual
wizard::UiResources::~UiResources() {}

// ============================================================================
//! Singleton instance
void wizard::UiResources::Create()
{
   assert(!sInstancePtr);
   sInstancePtr = new UiResources;
   sInstancePtr->Initialize();
}

// static
void wizard::UiResources::Destroy()
{
   delete sInstancePtr;
   sInstancePtr = nullptr;
}

// ============================================================================
void wizard::UiResources::Initialize()
{
   // Fonts
#ifdef _WIN32
   mConsoleFont = QFont("Consolas", 12);
#else
   mConsoleFont = QFont("Courier New", 12);
#endif
   mConsoleFont.setStyleHint(QFont::TypeWriter);
}

// ============================================================================
QIcon& wizard::UiResources::GetIcon(const QString& aImageName, bool aScale)
{
   QString                        iconName = QString(":/icons/%1.png").arg(aImageName);
   QMap<QString, QIcon>::iterator iter     = mIcons.find(iconName);
   if (iter != mIcons.end())
   {
      // Found
      return iter.value();
   }
   else
   {
      // Not Found; so add
      return AddIcon(iconName, aScale);
   }
}

// ============================================================================
QIcon& wizard::UiResources::GetIconFromTheme(const QString& aImageName, bool aScale)
{
   QString                        iconName = QString(":/icons/%1/64x64/%2.png").arg(QIcon::themeName(), aImageName);
   QMap<QString, QIcon>::iterator iter     = mIcons.find(iconName);
   if (iter != mIcons.end())
   {
      // Found
      return iter.value();
   }
   else
   {
      // Not Found; so add
      return AddIcon(iconName, aScale);
   }
}

// ============================================================================
QIcon wizard::UiResources::GetFileIcon(const QFileInfo& aInfo)
{
   return mIconProvider.icon(aInfo);
}

// ============================================================================
QImage& wizard::UiResources::GetImageFromTheme(const QString& aImageName, bool aScale)
{
   QString                         imageName = QString(":/icons/%1/64x64/%2.png").arg(QIcon::themeName(), aImageName);
   QMap<QString, QImage>::iterator iter      = mImages.find(imageName);
   if (iter != mImages.end())
   {
      // Found
      return iter.value();
   }
   else
   {
      // Not Found; so add
      return AddImage(imageName, aScale);
   }
}

// ============================================================================
QImage& wizard::UiResources::GetDimImageFromTheme(const QString& aImageName, bool aScale)
{
   QString                         imageName = QString(":/icons/%1/64x64/%2.png").arg(QIcon::themeName(), aImageName);
   QMap<QString, QImage>::iterator iter      = mDimImages.find(imageName);
   if (iter != mDimImages.end())
   {
      // Found
      return iter.value();
   }
   else
   {
      // Not Found; so add
      return AddDimImage(imageName, aScale);
   }
}

// ============================================================================
std::pair<unsigned int, unsigned int> wizard::UiResources::ResizeImageWithFont()
{
   unsigned int newSize    = cBASE_ICON_SIZE;
   unsigned int lineOffset = 0;
   auto         prefObj    = wkfEnv.GetPreferenceObject<wizard::EditorPrefObject>();
   if (prefObj && prefObj->FontSize() <= (cMAX_FONT_SIZE + cMIN_FONT_SIZE) / 2)
   {
      newSize = (cBASE_ICON_SIZE / 2) + (cBASE_ICON_SIZE) * (prefObj->FontSize() - cMIN_FONT_SIZE) /
                                           (cMAX_FONT_SIZE - cMIN_FONT_SIZE); // will vary between between full size and
                                                                              // half size icons based on font
      lineOffset = (cBASE_ICON_SIZE - newSize) / 2;                           // needed to recenter icon
   }
   return std::pair<unsigned int, unsigned int>(newSize, lineOffset);
}

// ============================================================================
QImage wizard::UiResources::ToScale(QImage aImage, int aWidth)
{
   return !aImage.isNull() ? aImage.scaled(QSize(aWidth, aWidth), Qt::IgnoreAspectRatio, Qt::SmoothTransformation) :
                             QImage();
}

QPixmap wizard::UiResources::ToScalePM(QImage aImage, int aWidth)
{
   return !aImage.isNull() ?
             QPixmap::fromImage(aImage.scaled(QSize(aWidth, aWidth), Qt::IgnoreAspectRatio, Qt::SmoothTransformation)) :
             QPixmap();
}

// ============================================================================
QIcon& wizard::UiResources::AddIcon(const QString& aIconName, bool aScale)
{
   QImage image(aIconName);
   if (!image.isNull())
   {
      if (aScale)
      {
         mIcons.insert(aIconName, QPixmap::fromImage(ToScale(image, cBASE_ICON_SIZE)));
      }
      else
      {
         mIcons.insert(aIconName, QPixmap::fromImage(image));
      }
      return mIcons.find(aIconName).value();
   }
   return mEmptyIcon;
}

// ============================================================================
QImage& wizard::UiResources::AddImage(const QString& aImageName, bool aScale)
{
   QImage image(aImageName);
   if (!image.isNull())
   {
      if (aScale)
      {
         mImages.insert(aImageName, ToScale(image, cBASE_ICON_SIZE));
      }
      else
      {
         mImages.insert(aImageName, image);
      }
      return mImages.find(aImageName).value();
   }
   return mEmptyImage;
}

// ============================================================================
QImage& wizard::UiResources::AddDimImage(const QString& aImageName, bool aScale)
{
   QImage image(aImageName);
   if (!image.isNull())
   {
      const QRgb rgbWhite = QColor(255, 255, 255).rgb();
      for (int x = 0; x < image.width(); ++x)
      {
         for (int y = 0; y < image.height(); ++y)
         {
            if ((x + y) % 2)
            {
               image.setPixel(QPoint(x, y), rgbWhite);
            }
         }
      }

      if (aScale)
      {
         mDimImages.insert(aImageName, ToScale(image, cBASE_ICON_SIZE));
      }
      else
      {
         mDimImages.insert(aImageName, image);
      }
      return mDimImages.find(aImageName).value();
   }
   return mEmptyImage;
}
