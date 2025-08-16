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

#include "ClonePlatformDialog.hpp"

#include <QMimeData>
#include <QStandardItemModel>

#include "DragAndDrop.hpp"
#include "PasteContext.hpp"
#include "ProxyWatcher.hpp"
#include "Signals.hpp"
#include "Undo.hpp"
#include "UtEllipsoidalEarth.hpp"
#include "UtEntity.hpp"
#include "UtSphericalEarth.hpp"
#include "UtVec3.hpp"
#include "WsfPM_Platform.hpp"
#include "WsfPM_Root.hpp"
#include "WsfPProxyIndex.hpp"
#include "WsfParseNode.hpp"

wizard::ClonePlatformDialog::ClonePlatformDialog(QWidget* aParentWindow)
   : QDialog(aParentWindow, Qt::Dialog)
   , mApplyToTextImmediately(false)
{
   setModal(true);
   mNamesValid = false;
   mUi.setupUi(this);
   mUi.offsetEast->SetUnitType(UtUnits::cLENGTH);
   mUi.offsetNorth->SetUnitType(UtUnits::cLENGTH);

   mModelPtr = new QStandardItemModel(this);
   mModelPtr->setHorizontalHeaderLabels(QStringList() << "Old Name"
                                                      << "New Name");

   mUi.nameList->setModel(mModelPtr);

   connect(mUi.suffixPrefixEdit, &QLineEdit::textEdited, this, &ClonePlatformDialog::GenerateNames);
   connect(mUi.manualOption, &QRadioButton::clicked, this, &ClonePlatformDialog::GenerateNames);
   connect(mUi.suffixOption, &QRadioButton::clicked, this, &ClonePlatformDialog::GenerateNames);
   connect(mUi.prefixOption, &QRadioButton::clicked, this, &ClonePlatformDialog::GenerateNames);
   connect(mModelPtr, &QStandardItemModel::itemChanged, this, &ClonePlatformDialog::ItemChanged);
   connect(&mDelayedValidationTimer, &QTimer::timeout, this, &ClonePlatformDialog::ValidateNames);
   connect(mUi.buttonBox, &QDialogButtonBox::accepted, this, &ClonePlatformDialog::accept);
   connect(mUi.buttonBox, &QDialogButtonBox::rejected, this, &ClonePlatformDialog::reject);
   connect(mUi.offsetNorth, &UtQtUnitValueEdit::ValueChanged, this, &ClonePlatformDialog::UpdateAcceptStatus);
   connect(mUi.offsetEast, &UtQtUnitValueEdit::ValueChanged, this, &ClonePlatformDialog::UpdateAcceptStatus);
   connect(mUi.offsetPosition, &QRadioButton::clicked, this, &ClonePlatformDialog::UpdateAcceptStatus);

   mDelayedValidationTimer.setInterval(100);
   mDelayedValidationTimer.setSingleShot(true);
}

//! Clone the platform with the new name
void wizard::ClonePlatformDialog::Clone(const WsfPM_Platform& aPlatform, const std::string& aName)
{
   bool copyRoutes    = mUi.copyRoutes->isChecked();
   bool fixCommanders = mUi.updateCommander->isChecked();
   bool doOffset      = mUi.offsetPosition->isChecked();
   bool setCenter     = mUi.centerOn->isChecked();

   WsfPM_PlatformMap platforms   = GetRoot().platforms();
   WsfPProxyValue    newPlatform = aPlatform.GetValue().Copy();
   if (fixCommanders)
   {
      WsfPProxyObjectMap* chainMapPtr = newPlatform["commandChains"].GetObjectMap();
      if (chainMapPtr)
      {
         std::vector<std::string> chainNames = chainMapPtr->GetObjectNames();
         for (size_t i = 0; i < chainNames.size(); ++i)
         {
            WsfPProxyValue cmdrNameValue = chainMapPtr->GetAt(chainNames[i]);
            std::string    cmdr          = cmdrNameValue.ValueToString();
            std::string    newName       = GetNewName(cmdr);
            if (cmdr != newName)
            {
               cmdrNameValue.SetValue(newName);
            }
         }
      }
   }

   double offsetNorth(0), offsetEast(0);
   if (doOffset)
   {
      UtLengthValue east, north;
      if (mUi.offsetNorth->GetValue(north) && mUi.offsetEast->GetValue(east))
      {
         offsetNorth = north;
         offsetEast  = east;
      }
      else
      {
      }
   }
   else if (setCenter)
   {
      double lat, lon, alt;
      aPlatform.InitialLocationLLA(lat, lon, alt);
      double heading, dist;
      UtSphericalEarth::GreatCircleHeadingAndDistance(lat,
                                                      lon,
                                                      mUi.latitudeCenter->GetValue(),
                                                      mUi.longitudeCenter->GetValue(),
                                                      heading,
                                                      dist);
      offsetNorth = cos(heading * UtMath::cRAD_PER_DEG) * dist;
      offsetEast  = sin(heading * UtMath::cRAD_PER_DEG) * dist;
   }


   if (!copyRoutes)
   {
      WsfPProxyValue routeValue = newPlatform["movers"]["mover"]["defaultRoute"];
      if (routeValue && !routeValue.IsUnset())
      {
         // Copy an empty route over the old one
         WsfPProxyValue emptyRoute = aPlatform.GetProxy()->NewValue("Route");
         emptyRoute.SetUnset();
         routeValue.Copy(emptyRoute);
         emptyRoute.Delete();
      }
   }
   else
   {
      WsfPProxyValue routeValue = newPlatform["movers"]["mover"]["defaultRoute"];
      if (routeValue && !routeValue.IsUnset())
      {
         // try to transform the route
         WsfPProxyList* waypointsListPtr = routeValue["waypoints"].GetList();
         if (waypointsListPtr)
         {
            for (size_t i = 0; i < waypointsListPtr->Size(); ++i)
            {
               WsfPProxyValue waypoint = waypointsListPtr->Get(i);
               int            wptType  = -1;
               if (waypoint["waypointType"].GetIntegerValue(wptType))
               {
                  if (wptType == 0)
                  {
                     if (offsetNorth != 0 || offsetEast != 0)
                     {
                        WsfProxy::Position* pos = (WsfProxy::Position*)waypoint["position"].GetDataPtr();
                        if (pos)
                        {
                           UtEntity tmp;
                           double   alt = 0;
                           tmp.SetLocationLLA(pos->GetLatitude(), pos->GetLongitude(), alt);
                           double ned[] = {offsetNorth, offsetEast, 0};
                           tmp.IncrementLocationNED(ned);
                           double lat, lon;
                           tmp.GetLocationLLA(lat, lon, alt);
                           waypoint["position"] = WsfProxy::Position(UtLatPos(lat), UtLonPos(lon));
                        }
                     }
                  }
               }
            }
         }
      }
   }

   if (doOffset)
   {
      double lat = 0, lon = 0, alt = 0;
      aPlatform.InitialLocationLLA(lat, lon, alt);
      UtEntity tmp;
      tmp.SetLocationLLA(lat, lon, alt);
      UtLengthValue east, north;
      if (mUi.offsetNorth->GetValue(north) && mUi.offsetEast->GetValue(east))
      {
         double ned[] = {north, east, 0};
         tmp.IncrementLocationNED(ned);
         tmp.GetLocationLLA(lat, lon, alt);
         newPlatform["position"] = WsfProxy::Position(UtLatPos(lat), UtLonPos(lon));
      }
   }
   else if (setCenter)
   {
      double lat, lon, alt;
      aPlatform.InitialLocationLLA(lat, lon, alt);
      double heading, dist;
      UtSphericalEarth::GreatCircleHeadingAndDistance(mCentroidLat, mCentroidLon, lat, lon, heading, dist);
      UtSphericalEarth::ExtrapolateGreatCirclePosition(mUi.latitudeCenter->GetValue(),
                                                       mUi.longitudeCenter->GetValue(),
                                                       heading,
                                                       dist,
                                                       lat,
                                                       lon);
      newPlatform["position"] = WsfProxy::Position(UtLatPos(lat), UtLonPos(lon));
   }

   platforms.Add(aName, newPlatform);

   // Save cloned platform to the right file...
   {
      WsfPProxyNode       newPlatformNode = platforms + aName;
      WsfPProxyIndexNode* indexPtr        = aPlatform.FindIndexNode();
      std::string         filePath        = mDestinationFilePath;
      if (filePath.empty() && indexPtr && !indexPtr->mEntries.empty())
      {
         WsfParseNode* nodePtr = indexPtr->mEntries[0].mNodePtr ? indexPtr->mEntries[0].mNodePtr->NextLeaf() : nullptr;
         if (nodePtr)
         {
            filePath = nodePtr->mValue.mSource->GetFilePath().GetSystemPath();
         }
      }
      // If it's not saved yet...
      if (filePath.empty())
      {
         const std::map<WsfPProxyPath, std::string>& saveMap = aPlatform.GetProxy()->SaveFiles();
         if (saveMap.find(newPlatformNode.GetPath()) != saveMap.end())
         {
            filePath = saveMap.find(newPlatformNode.GetPath())->second;
         }
      }
      if (!filePath.empty())
      {
         newPlatformNode.GetProxy()->SetSaveFile(newPlatformNode.GetPath(), filePath);
      }
   }
}

//! Given a platform name, return it's new name if it belongs to the cloned group.  Otherwise return the old name
std::string wizard::ClonePlatformDialog::GetNewName(const std::string& aOldName)
{
   for (int i = 0; i < (int)mPlatforms.size() && i < mModelPtr->rowCount(); ++i)
   {
      if (mModelPtr->item(i, 0)->text().toStdString() == aOldName)
      {
         return mModelPtr->item(i, 1)->text().toStdString();
      }
   }
   return aOldName;
}

void wizard::ClonePlatformDialog::accept()
{
   bool inVisualMode = wizSignals->InVisualMode();

   std::stringstream description;
   description << "Clone Platforms (" << mPlatforms.size() << ")";
   wizard::UndoContext ctx(description.str().c_str());

   UtVec3d wcsSum;
   for (int i = 0; i < (int)mPlatforms.size(); ++i)
   {
      double platLat, platLon, platAlt;
      mPlatforms[i].InitialLocationLLA(platLat, platLon, platAlt);
      UtVec3d wcs;
      UtEllipsoidalEarth::ConvertLLAToECEF(platLat, platLon, 0, wcs.GetData());
      wcsSum += wcs;
   }
   wcsSum /= (double)mPlatforms.size();
   double alt;
   UtEllipsoidalEarth::ConvertECEFToLLA(wcsSum.GetData(), mCentroidLat, mCentroidLon, alt);

   for (int i = 0; i < (int)mPlatforms.size() && i < mModelPtr->rowCount(); ++i)
   {
      std::string name = mModelPtr->item(i, 1)->text().toStdString();
      Clone(mPlatforms[i], name);
   }

   // If we started in text mode, go back to text mode
   if (!inVisualMode && mApplyToTextImmediately)
   {
      wizSignals->applyProxy();
   }

   QDialog::accept();
   deleteLater();
}

void wizard::ClonePlatformDialog::reject()
{
   QDialog::reject();
   deleteLater();
}

void wizard::ClonePlatformDialog::GenerateNames()
{
   std::string spt    = mUi.suffixPrefixEdit->text().toStdString();
   bool        prefix = mUi.prefixOption->isChecked();
   bool        suffix = mUi.suffixOption->isChecked();
   bool        manual = mUi.manualOption->isChecked();
   mUi.suffixPrefixEdit->setEnabled(!manual);
   if (prefix)
      mUi.suffixOrPrefixLabel->setText("Prefix:");
   if (suffix)
      mUi.suffixOrPrefixLabel->setText("Suffix:");
   for (int i = 0; i < mModelPtr->rowCount(); ++i)
   {
      QStandardItem* itemPtr = mModelPtr->item(i, 0);
      std::string    name    = itemPtr->text().toStdString();
      if (suffix)
         name = name + spt;
      if (prefix)
         name = spt + name;
      mModelPtr->item(i, 1)->setText(name.c_str());
   }
}

void wizard::ClonePlatformDialog::Show(const std::vector<WsfPM_Platform>& aPlatforms)
{
   mModelPtr->clear();
   mPlatforms = aPlatforms;

   for (size_t i = 0; i < aPlatforms.size(); ++i)
   {
      const WsfPM_Platform& p = aPlatforms[i];
      QList<QStandardItem*> row;
      QStandardItem*        src = new QStandardItem(p.GetName().c_str());
      src->setFlags(0);
      QStandardItem* newName = new QStandardItem();
      newName->setFlags(Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
      mModelPtr->appendRow(row << src << newName);
   }
   GenerateNames();
   show();
}

void wizard::ClonePlatformDialog::ItemChanged(QStandardItem* aItemPtr)
{
   if (aItemPtr->column() == 1)
   {
      mNamesValid = false;
      mDelayedValidationTimer.start();
   }
}

void wizard::ClonePlatformDialog::ValidateNames()
{
   WsfPM_PlatformMap platforms = GetRoot().platforms();
   mNamesValid                 = true;
   std::set<std::string> usedNames;
   for (int i = 0; i < mModelPtr->rowCount(); ++i)
   {
      QStandardItem* itemPtr = mModelPtr->item(i, 1);
      std::string    name    = itemPtr->text().toStdString();
      bool           valid   = !platforms.Exists(name) && usedNames.find(name) == usedNames.end();
      if (!valid)
      {
         itemPtr->setForeground(Qt::red);
         mNamesValid = false;
      }
      else
      {
         itemPtr->setForeground(qApp->palette().color(QPalette::Text));
         usedNames.insert(name);
      }
   }
   UpdateAcceptStatus();
}

WsfPProxy* wizard::ClonePlatformDialog::GetProxy()
{
   if (mPlatforms.empty())
      return nullptr;
   return mPlatforms[0].GetProxy();
}

WsfPM_Root wizard::ClonePlatformDialog::GetRoot()
{
   return WsfPM_Root(GetProxy());
}

void wizard::ClonePlatformDialog::UpdateAcceptStatus()
{
   bool doingOffset    = mUi.offsetPosition->isChecked();
   bool doingSetCenter = mUi.centerOn->isChecked();
   bool noMove         = !doingOffset && !doingSetCenter;
   mUi.offsetNorth->setEnabled(doingOffset);
   mUi.offsetEast->setEnabled(doingOffset);
   bool valid =
      mNamesValid && ((doingOffset && mUi.offsetNorth->IsValid() && mUi.offsetEast->IsValid()) ||
                      (doingSetCenter && mUi.latitudeCenter->IsValid() && mUi.longitudeCenter->IsValid()) || noMove);
   mUi.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(valid);
}

void wizard::ClonePlatformDialog::SetReferenceLocation(const UtLatPos& aLat, const UtLonPos& aLon)
{
   mUi.latitudeCenter->SetValue(aLat);
   mUi.longitudeCenter->SetValue(aLon);
   mUi.centerOn->setChecked(true);
}

void wizard::ClonePlatformDialog::PasteHandler(const QMimeData* aMime, const PasteContext& aContext, bool aExecute)
{
   if (aExecute && !aContext.IsActionChosen("clone-platforms"))
      return;
   if (!aMime || !aMime->hasFormat(wizard::DragAndDrop::cPROPERTY_SET_MIME_TYPE))
      return;
   if (!wizard::ProxyWatcher::GetActiveProxy())
      return;
   wizard::DragAndDrop::PropertyList properties = wizard::DragAndDrop::ReadPropertySetMime(aMime);
   if (properties.empty())
      return;
   WsfPM_Root                  proxyRoot(wizard::ProxyWatcher::GetActiveProxy());
   std::vector<WsfPM_Platform> selectedPlatforms;
   WsfPM_PlatformMap           platformRoot = proxyRoot.platforms();
   for (int i = 0; i < properties.size(); ++i)
   {
      wizard::DragAndDrop::PropertyMap pMap = properties[i];
      if (pMap.contains("proxy-node"))
      {
         WsfPProxyNode node = wizard::DragAndDrop::ToProxyNode(pMap["proxy-node"].toString());
         if (node && node.GetParent() == platformRoot)
         {
            selectedPlatforms.push_back(node);
         }
      }
   }

   // On paste of platforms, initiate clone
   if (!selectedPlatforms.empty())
   {
      if (aExecute)
      {
         ClonePlatformDialog* dlg = new ClonePlatformDialog(aContext.GetWidget());
         if (aContext.HasPosition())
         {
            dlg->SetReferenceLocation(aContext.GetLat(), aContext.GetLon());
         }
         dlg->SetDestinationFilePath(aContext.GetFilePath());
         dlg->mApplyToTextImmediately = aContext.ApplyToTextImmediately();
         dlg->Show(selectedPlatforms);
      }
      else
      {
         QString label = "Platforms(";
         label += QString::number((int)selectedPlatforms.size()) + ")";
         aContext.AddAvailableAction("clone-platforms", label);
      }
   }
}
