// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2016 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef PATTERN_HPP
#define PATTERN_HPP

#include <memory>

#include <QObject>
#include <QString>

#include "PatternData.hpp"
#include "WsfScenario.hpp"

namespace PatternVisualizer
{
class Session;
class UpdatePatternDataThread;
class PatternUpdateManager;

class Pattern : public QObject
{
   Q_OBJECT

public:
   enum Type
   {
      ANTENNA_PAT  = 0,
      RADAR_SIG    = 1,
      OPTICAL_SIG  = 2,
      INFRARED_SIG = 3,
      ACOUSTIC_SIG = 4
   };

   Pattern(const QString& aFileName, Session* aSessionPtr, const QString& aName, Type aPatternType);
   ~Pattern() override = default;

   Pattern()               = delete;
   Pattern(const Pattern&) = delete;
   Pattern(Pattern&&)      = delete;
   Pattern& operator=(const Pattern&) = delete;

   constexpr static WsfEM_Types::Polarization DefaultPolarization = {WsfEM_Types::cPOL_DEFAULT};
   constexpr static WsfEM_Types::InfraredBand DefaultIRBand       = {WsfEM_Types::cIRBAND_DEFAULT};

   QString         GetFileName() const { return mFileName; }
   virtual QString GetName() const { return mName; }
   float           GetMinDB() const { return mPatternData->GetMinDB(); }
   float           GetMaxDB() const { return mPatternData->GetMaxDB(); }
   float           GetMinDBAllFreqs() const { return mPatternData->GetMinDBAllFreqs(); }
   float           GetMaxDBAllFreqs() const { return mPatternData->GetMaxDBAllFreqs(); }
   PatternData*    GetPatternData() const { return mPatternData.get(); }
   virtual bool    UpdatePatternData(double aFreq) = 0;

protected:
   Pattern::Type                mPatternType;
   QString                      mFileName;
   QString                      mName;
   std::unique_ptr<PatternData> mPatternData;

   static PatternUpdateManager* sPatternUpdateManager;
};

class AntPattern : public Pattern
{
public:
   AntPattern(const QString& aFileName, WsfAntennaPattern& aWsfPattern, Session* aSessionPtr, const QString& aPatternName);
   ~AntPattern() override = default;

   bool               UpdatePatternData(double aFreq) override;
   WsfAntennaPattern& GetPattern() { return mWsfPattern; }

private:
   WsfAntennaPattern& mWsfPattern;
};

class RadarSigPattern : public Pattern
{
public:
   RadarSigPattern(const QString&            aFileName,
                   WsfRadarSignature&        aWsfPattern,
                   Session*                  aSessionPtr,
                   const QString&            aSigName,
                   const QString&            aState,
                   WsfEM_Types::Polarization aPolarization);
   ~RadarSigPattern() override = default;

   bool               UpdatePatternData(double aFreq) override;
   WsfRadarSignature& GetPattern() { return mWsfPattern; }

   static QStringList sPolLabels;

private:
   WsfRadarSignature&        mWsfPattern;
   WsfStringId               mState;
   WsfEM_Types::Polarization mPolarization;
};

class OpticalSigPattern : public Pattern
{
public:
   OpticalSigPattern(const QString&       aFileName,
                     WsfOpticalSignature& aWsfPattern,
                     Session*             aSessionPtr,
                     const QString&       aSigName,
                     const QString&       aState);
   ~OpticalSigPattern() override = default;

   bool                 UpdatePatternData(double aFreq) override;
   WsfOpticalSignature& GetPattern() { return mWsfPattern; }

private:
   WsfOpticalSignature& mWsfPattern;
   WsfStringId          mState;
};

class InfraredSigPattern : public Pattern
{
public:
   InfraredSigPattern(const QString&        aFileName,
                      WsfInfraredSignature& aWsfPattern,
                      Session*              aSessionPtr,
                      const QString&        aSigName,
                      const QString&        aState);
   ~InfraredSigPattern() override = default;

   bool                             UpdatePatternData(double aFreq) override;
   WsfInfraredSignature&            GetPattern() { return mWsfPattern; }
   static WsfEM_Types::InfraredBand GetBand(double aFreq);

private:
   WsfInfraredSignature& mWsfPattern;
   WsfStringId           mState;
};

class AcousticSigPattern : public Pattern
{
public:
   AcousticSigPattern(const QString&        aFileName,
                      WsfAcousticSignature& aWsfPattern,
                      Session*              aSessionPtr,
                      const QString&        aSigName,
                      const QString&        aState);
   ~AcousticSigPattern() override = default;

   bool                  UpdatePatternData(double aFreq) override;
   WsfAcousticSignature& GetPattern() { return mWsfPattern; }

private:
   WsfAcousticSignature& mWsfPattern;
   WsfStringId           mState;
};
} // namespace PatternVisualizer

#endif
