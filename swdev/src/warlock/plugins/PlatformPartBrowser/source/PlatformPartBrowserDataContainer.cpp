// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2018 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "PlatformPartBrowserDataContainer.hpp"

#include <QCheckBox>

#include "WkfUnitTypes.hpp"

void WkPlatformPartBrowser::DataContainer::SetPlatformOfInterest(const std::string& aPlatformName)
{
   mPlatformOfInterest = aPlatformName;
   mPlatformParts.clear();
   mTreeWidgets.clear(); // These should be deleted by the DockWidget
}

void WkPlatformPartBrowser::DataContainer::SetParts(const std::string& aPlatformName, const PlatformPartList& aPlatformParts)
{
   if (aPlatformName == mPlatformOfInterest)
   {
      mPlatformParts = aPlatformParts;
      emit PlatformPartsSet(QString::fromStdString(mPlatformOfInterest), mPlatformParts);
   }
}

void WkPlatformPartBrowser::DataContainer::CreateTreeWidgetItems(QTreeWidget* aTreeWidget)
{
   for (const auto& part : mPlatformParts)
   {
      QTreeWidgetItem* partItem =
         new QTreeWidgetItem(QStringList() << part.GetName().c_str() << part.GetUserType().c_str());

      partItem->setIcon(0, QIcon::fromTheme(sPartTypeToIcon.at(part.GetType())));
      aTreeWidget->addTopLevelItem(partItem);

      for (const auto& pProperty : part.GetProperties())
      {
         mTreeWidgets[part.GetUniqueId()][pProperty.first] = CreateItem(part, pProperty);
      }

      QCheckBox* onItem        = nullptr;
      bool       isOperational = true;

      for (const auto& pair : mTreeWidgets[part.GetUniqueId()])
      {
         for (const auto& widget : pair.second)
         {
            partItem->addChild(widget);
            // Some QTreeWidgetItem have Item Widgets set within them (such as QCheckBox)
            // This requires the item to already be part of the QTreeWidget, and because we
            // create the items without a parent and then add them to the QTreeWidget, the Item Widgets
            // such as CheckBoxes must be done after the QTreeWidgetItem is added to the QTreeWidget
            // Therefore, check for the special case here
            if (widget->type() == eCHECKBOX)
            {
               PropertyName propName = static_cast<PropertyName>(widget->data(0, Qt::UserRole).toInt());
               AddCheckBoxToTreeWidgetItem(aTreeWidget, widget, part, propName);

               if (propName == eON)
               {
                  onItem = dynamic_cast<QCheckBox*>(widget->treeWidget()->itemWidget(widget, 1));
               }
               else if (propName == eOPERATIONAL)
               {
                  isOperational = part.FindProperty(eOPERATIONAL).toBool();
               }
            }
         }
      }

      if (onItem != nullptr && !isOperational)
      {
         onItem->setEnabled(false);
      }
   }
}

QList<QTreeWidgetItem*> WkPlatformPartBrowser::DataContainer::CreateItem(const PlatformPart& aPart, const Property& aProperty)
{
   QList<QTreeWidgetItem*> items;

   // QTreeWidgetItem* item = nullptr;
   switch (aProperty.first)
   {
   // Check box
   case eDEBUG:
   case eON:
   case eOPERATIONAL:
   {
      QTreeWidgetItem* item = new QTreeWidgetItem(eCHECKBOX); // Set Type to CheckBox, so it can be added later
      item->setData(0, Qt::UserRole, aProperty.first);        // Store PropertyName for use when adding a QCheckBox
      item->setText(0, sPropertyNameToString.at(aProperty.first));
      items.push_back(item);
      break;
   }
   // String
   case eBROKEN:
   case eSLEWMODE:
   case eCURRENTMODE:
   {
      QTreeWidgetItem* item = new QTreeWidgetItem();
      item->setText(0, sPropertyNameToString.at(aProperty.first));
      item->setText(1, aProperty.second.toString());
      items.push_back(item);
      break;
   }
   // Angle
   case eROLL:
   case ePITCH:
   case eYAW:
   case eTILT:
   {
      wkf::AngleTreeWidgetItem* angleItem =
         new wkf::AngleTreeWidgetItem(nullptr, sPropertyNameToString.at(aProperty.first));
      angleItem->SetValue(aProperty.second.toDouble());
      items.push_back(angleItem);
      break;
   }
   // StringList
   case eMODES:
   {
      QTreeWidgetItem* item = new QTreeWidgetItem();
      item->setText(0, sPropertyNameToString.at(aProperty.first));

      QStringList list       = aProperty.second.toStringList();
      QString     listString = list.join(", ");

      if (listString.isEmpty())
      {
         listString = "none";
      }

      item->setText(1, listString);
      items.push_back(item);
      break;
   }
   case eTRACKS:
   {
      QTreeWidgetItem* item = new QTreeWidgetItem();
      item->setText(0, sPropertyNameToString.at(aProperty.first));

      QStringList list = aProperty.second.toStringList();
      for (const auto& track : list)
      {
         QTreeWidgetItem* child = new QTreeWidgetItem();
         child->setText(1, track);
         item->addChild(child);
      }

      if (list.empty())
      {
         QTreeWidgetItem* child = new QTreeWidgetItem();
         child->setText(1, "none");

         QFont font = child->font(1);
         font.setItalic(true);
         child->setFont(1, font);

         item->addChild(child);
      }
      items.push_back(item);
      break;
   }
   // Double List
   case eRECEIVERS:
   {
      QList<QVariant> receiverList = aProperty.second.toList();
      for (auto& receiver : receiverList)
      {
         QTreeWidgetItem* item = new QTreeWidgetItem();
         item->setText(0, sPropertyNameToString.at(aProperty.first));
         QList<QVariant> receiverProperties = receiver.toList();
         for (auto& receiverProperty : receiverProperties)
         {
            Property p = receiverProperty.value<WkPlatformPartBrowser::Property>();
            switch (p.first)
            {
            case eBANDWIDTH:
            {
               wkf::FrequencyTreeWidgetItem* bandwidthItem =
                  new wkf::FrequencyTreeWidgetItem(item, sPropertyNameToString.at(p.first));
               bandwidthItem->SetValue(p.second.toDouble());
               item->addChild(bandwidthItem); // This is not required as it does nothing since it is already a child,
                                              // but CppCheck complains
               break;
            }
            case eNOISEPOWER:
            {
               wkf::PowerDBTreeWidgetItem* noisePowerItem =
                  new wkf::PowerDBTreeWidgetItem(item, sPropertyNameToString.at(p.first));
               noisePowerItem->SetValue(p.second.toDouble());
               item->addChild(noisePowerItem); // This is not required as it does nothing since it is already a child,
                                               // but CppCheck complains
               break;
            }
            case eDETECTIONTHRESHOLD:
            {
               wkf::RatioTreeWidgetItem* detectionThresholdItem =
                  new wkf::RatioTreeWidgetItem(item, sPropertyNameToString.at(p.first));
               detectionThresholdItem->SetValue(p.second.toDouble());
               item->addChild(detectionThresholdItem); // This is not required as it does nothing since it is already a
                                                       // child, but CppCheck complains
               break;
            }
            case eRANGELIMITSMIN:
            case eRANGELIMITSMAX:
            {
               double range = p.second.toDouble();
               if (range == std::numeric_limits<double>::max())
               {
                  QTreeWidgetItem* rangeItem = new QTreeWidgetItem(item);
                  rangeItem->setText(0, sPropertyNameToString.at(p.first));
                  rangeItem->setText(1, "INF");
                  item->addChild(rangeItem); // This is not required as it does nothing since it is already a child, but
                                             // CppCheck complains
               }
               else
               {
                  wkf::LengthTreeWidgetItem* rangeItem =
                     new wkf::LengthTreeWidgetItem(item, sPropertyNameToString.at(p.first));
                  rangeItem->SetValue(range);
                  item->addChild(rangeItem); // This is not required as it does nothing since it is already a child, but
                                             // CppCheck complains
               }
               break;
            }
            case eELEVATIONFOVMIN:
            case eELEVATIONFOVMAX:
            case eAZIMUTHFOVMIN:
            case eAZIMUTHFOVMAX:
            {
               wkf::AngleTreeWidgetItem* angleItem = new wkf::AngleTreeWidgetItem(item, sPropertyNameToString.at(p.first));
               angleItem->SetValue(p.second.toDouble());
               item->addChild(angleItem); // This is not required as it does nothing since it is already a child, but
                                          // CppCheck complains
               break;
            }
            default:
               break;
            }
         }
         items.push_back(item);
      }
      break;
   }
   case eTRANSMITTERS:
   {
      QList<QVariant> transmitterList = aProperty.second.toList();
      for (auto& transmitter : transmitterList)
      {
         QTreeWidgetItem* item = new QTreeWidgetItem();
         item->setText(0, sPropertyNameToString.at(aProperty.first));
         QList<QVariant> transmitterProperties = transmitter.toList();
         for (auto& transmitterProperty : transmitterProperties)
         {
            Property p = transmitterProperty.value<WkPlatformPartBrowser::Property>();
            switch (p.first)
            {
            case ePEAKPOWER:
            case eAVERAGEPOWER:
            {
               wkf::PowerTreeWidgetItem* powerItem = new wkf::PowerTreeWidgetItem(item, sPropertyNameToString.at(p.first));
               powerItem->SetValue(p.second.toDouble());
               item->addChild(powerItem); // This is not required as it does nothing since it is already a child, but
                                          // CppCheck complains
               break;
            }
            case eFREQUENCY:
            {
               wkf::FrequencyTreeWidgetItem* frequencyItem =
                  new wkf::FrequencyTreeWidgetItem(item, sPropertyNameToString.at(p.first));
               frequencyItem->SetValue(p.second.toDouble());
               item->addChild(frequencyItem); // This is not required as it does nothing since it is already a child,
                                              // but CppCheck complains
               break;
            }
            case ePULSEWIDTH:
            {
               wkf::TimeTreeWidgetItem* timeItem = new wkf::TimeTreeWidgetItem(item, sPropertyNameToString.at(p.first));
               timeItem->SetValue(p.second.toDouble());
               item->addChild(timeItem); // This is not required as it does nothing since it is already a child, but
                                         // CppCheck complains
               break;
            }
            case ePRI:
            {
               double pri = p.second.toDouble();
               if (!std::isnan(pri))
               {
                  wkf::TimeTreeWidgetItem* timeItem = new wkf::TimeTreeWidgetItem(item, sPropertyNameToString.at(p.first));
                  timeItem->SetValue(pri);
                  item->addChild(timeItem); // This is not required as it does nothing since it is already a child, but
                                            // CppCheck complains
                  break;
               }
            }
            default:
               break;
            }
         }
         items.push_back(item);
      }
      break;
   }
   default:
      break;
   };

   return items;
}

void WkPlatformPartBrowser::DataContainer::UpdateItem(unsigned int    aUniqueId,
                                                      PropertyName    aPropertyName,
                                                      const QVariant& aVariant)
{
   auto idIter = mTreeWidgets.find(aUniqueId);
   if (idIter != mTreeWidgets.end())
   {
      auto propIt = idIter->second.find(aPropertyName);
      if (propIt != idIter->second.end())
      {
         QList<QTreeWidgetItem*> items = propIt->second;
         for (auto& item : items)
         {
            if (item)
            {
               // Only Properties that may have to be updated appear in this list.
               // Static data on a PlatformPart will not have to be updated.
               switch (aPropertyName)
               {
               case eDEBUG:
               case eON:
               {
                  QCheckBox* checkBox = dynamic_cast<QCheckBox*>(item->treeWidget()->itemWidget(item, 1));
                  if (checkBox)
                  {
                     checkBox->setChecked(aVariant.toBool());
                  }
                  break;
               }
               case eOPERATIONAL:
               {
                  QCheckBox* checkBox = dynamic_cast<QCheckBox*>(item->treeWidget()->itemWidget(item, 1));
                  if (checkBox)
                  {
                     checkBox->setChecked(aVariant.toBool());

                     // set enabled state of 'on' CheckBox based on checked state of operational
                     auto onIter = idIter->second.find(eON);
                     if (onIter != idIter->second.end())
                     {
                        QTreeWidgetItem* onItem = onIter->second[0];
                        if (onItem != nullptr)
                        {
                           QWidget* onWidget = onItem->treeWidget()->itemWidget(onItem, 1);
                           if (onWidget)
                           {
                              onWidget->setEnabled(aVariant.toBool());
                           }
                        }
                     }
                  }
                  break;
               }
               case eYAW:
               case ePITCH:
               case eROLL:
               case eTILT:
               {
                  wkf::AngleTreeWidgetItem* angleItem = dynamic_cast<wkf::AngleTreeWidgetItem*>(item);
                  if (angleItem)
                  {
                     angleItem->SetValue(aVariant.toDouble());
                  }
                  break;
               }
               case eCURRENTMODE:
               {
                  item->setText(1, aVariant.toString());
                  break;
               }
               case eTRACKS:
               {
                  QStringList trackList = aVariant.toStringList();

                  QList<QTreeWidgetItem*> oldTracks = item->takeChildren();
                  for (auto& old : oldTracks)
                  {
                     delete old;
                  }

                  for (const auto& track : trackList)
                  {
                     QTreeWidgetItem* child = new QTreeWidgetItem();
                     child->setText(1, track);
                     item->addChild(child);
                  }
                  if (trackList.empty())
                  {
                     QTreeWidgetItem* child = new QTreeWidgetItem();
                     child->setText(1, "none");

                     QFont font = child->font(1);
                     font.setItalic(true);
                     child->setFont(1, font);

                     item->addChild(child);
                  }
                  break;
               }
               // Double List
               case eTRANSMITTERS:
               {
                  QList<QVariant> transmitters = aVariant.toList(); // get the list of all transmitters for this part
                  if (!transmitters.empty())
                  {
                     unsigned int i = items.indexOf(item); // Note: We assume that the tree widget items are in the same
                                                           // order as the items in the QList
                     if (i >= (unsigned int)transmitters.size())
                     {
                        // error case, should we print a message here?
                     }
                     else
                     {
                        const QVariant& transmitter =
                           transmitters.at(i); // get the transmitter associated with the item we're updating
                        QList<QVariant> properties =
                           transmitter.toList(); // get the list of properties for this transmitter

                        unsigned int childCount = item->childCount();

                        for (unsigned int c = 0; c < childCount; ++c)
                        {
                           QTreeWidgetItem* propItem = item->child(c);

                           for (auto& prop : properties)
                           {
                              Property p = prop.value<Property>();
                              if (propItem->text(0) == sPropertyNameToString.at(p.first))
                              {
                                 wkf::UnitTreeWidgetItem* unitItem = dynamic_cast<wkf::UnitTreeWidgetItem*>(propItem);
                                 if (unitItem)
                                 {
                                    unitItem->SetValue(p.second.toDouble());
                                 }
                                 else
                                 {
                                    propItem->setText(1, p.second.toString());
                                 }
                                 break; // we found the right property, stop searching
                              }
                           }
                        }
                     }
                  }
                  break;
               }
               case eRECEIVERS:
               {
                  QList<QVariant> receivers = aVariant.toList(); // get the list of all receivers for this part
                  if (!receivers.empty())
                  {
                     unsigned int i = items.indexOf(item); // Note: We assume that the tree widget items are in the same
                                                           // order as the items in the QList
                     if (i >= (unsigned int)receivers.size())
                     {
                        // error case, should we print a message here?
                     }
                     else
                     {
                        const QVariant& receiver =
                           receivers.at(i); // get the receiver associated with the item we're updating
                        QList<QVariant> properties = receiver.toList(); // get the list of properties for this receiver

                        unsigned int childCount = item->childCount();

                        for (unsigned int c = 0; c < childCount; ++c)
                        {
                           QTreeWidgetItem* propItem = item->child(c);

                           for (auto& prop : properties)
                           {
                              Property p = prop.value<Property>();

                              if (propItem->text(0) == sPropertyNameToString.at(p.first))
                              {
                                 if (p.first == eRANGELIMITSMAX)
                                 {
                                    // if the range is set to max value, set text of TreeWidgetItem
                                    if (p.second.toDouble() == std::numeric_limits<double>::max())
                                    {
                                       propItem->setText(1, "INF");
                                    }
                                    else
                                    {
                                       wkf::UnitTreeWidgetItem* unitItem =
                                          dynamic_cast<wkf::UnitTreeWidgetItem*>(propItem);
                                       if (unitItem !=
                                           nullptr) // if the range is not inf and the item is already a unit item
                                       {
                                          unitItem->SetValue(p.second.toDouble());
                                       }
                                       else
                                       {
                                          // if the range is not inf, but the item is not a unit item,
                                          // we need to delete the treewidgetitem, and put a unit item in its place
                                          item->takeChild(c);

                                          wkf::LengthTreeWidgetItem* lengthItem =
                                             new wkf::LengthTreeWidgetItem(nullptr, "Range, Max");
                                          lengthItem->SetValue(p.second.toDouble());
                                          item->insertChild(c, lengthItem);
                                       }
                                    }
                                 }
                                 else
                                 {
                                    wkf::UnitTreeWidgetItem* unitItem = dynamic_cast<wkf::UnitTreeWidgetItem*>(propItem);
                                    if (unitItem != nullptr)
                                    {
                                       unitItem->SetValue(p.second.toDouble());
                                    }
                                    else
                                    {
                                       propItem->setText(1, p.second.toString());
                                    }
                                 }
                                 break; // we found the right property, stop searching
                              }
                           }
                        }
                     }
                  }
                  break;
               }
               default:
                  break;
               };
            }
         }
      }
   }
}

void WkPlatformPartBrowser::DataContainer::AddCheckBoxToTreeWidgetItem(QTreeWidget*        aTreeWidget,
                                                                       QTreeWidgetItem*    aItem,
                                                                       const PlatformPart& aPart,
                                                                       PropertyName        aPropertyName)
{
   QCheckBox* checkBox = new QCheckBox();
   checkBox->setChecked(aPart.FindProperty(aPropertyName).toBool());

   aTreeWidget->setItemWidget(aItem, 1, checkBox); // QTreeWidget takes ownership of QCheckBox

   // When the check box is modified, update the Part
   const std::string& partName = aPart.GetName();
   connect(checkBox,
           &QCheckBox::clicked,
           this,
           [=](bool aChecked) { emit CheckBoxToggled(partName, aPropertyName, aChecked); });
}

void WkPlatformPartBrowser::DataContainer::UpdateProperty(unsigned int    aUniqueId,
                                                          PropertyName    aPropertyName,
                                                          const QVariant& aVariant)
{
   for (auto& p : mPlatformParts)
   {
      if (p.GetUniqueId() == aUniqueId)
      {
         if (p.UpdateProperty(aPropertyName, aVariant))
         {
            UpdateItem(aUniqueId, aPropertyName, aVariant);
         }
      }
   }
}

void WkPlatformPartBrowser::DataContainer::UpdateParts(std::vector<PlatformPart>& aPlatformParts)
{
   if (!mPlatformOfInterest.empty()) // If there is no platform of interest, don't bother updating
   {
      for (auto& part : aPlatformParts)
      {
         auto oldPart = std::find(mPlatformParts.begin(), mPlatformParts.end(), part);
         if (oldPart != mPlatformParts.end())
         {
            for (auto& p : part.GetProperties())
            {
               if (oldPart->FindProperty(p.first) != p.second) // if the property changed, update
               {
                  UpdateItem(part.GetUniqueId(), p.first, p.second);
               }
            }
         }
      }

      mPlatformParts = aPlatformParts;
   }
}
