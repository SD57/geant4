//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
//
// $Id: G4TrackingManager.cc 77241 2013-11-22 09:55:47Z gcosmo $
//
//---------------------------------------------------------------
//
// G4TrackingManager.cc
//
// Contact:
//   Questions and comments to this code should be sent to
//     Katsuya Amako  (e-mail: Katsuya.Amako@kek.jp)
//     Takashi Sasaki (e-mail: Takashi.Sasaki@kek.jp)
//
//---------------------------------------------------------------

#include "G4TrackingManager.hh"
#include "G4Trajectory.hh"
#include "G4SmoothTrajectory.hh"
#include "G4RichTrajectory.hh"
#include "G4ios.hh"
#include "Randomize.hh"
#include "G4Philox.h"
#include "G4Threefry.h"

#include "CLHEP/Random/RandomEngine.h" // FIXME for the clang-based IDE parser, redundant

//#define G4TRACKINGMANAGERDEBUG

namespace
{
#ifdef G4TRACKINGMANAGERDEBUG
constexpr bool debugOutput = true;
#else
constexpr bool debugOutput = false;
#endif
} // anonymous namespace

namespace
{
template <class T> void hash_combine(std::size_t& seed, T const& v);
}

class G4VSteppingVerbose;

//////////////////////////////////////
G4TrackingManager::G4TrackingManager()
//////////////////////////////////////
  : fpUserTrackingAction(0), fpTrajectory(0),
    StoreTrajectory(0), verboseLevel(0), EventIsAborted(false),
    fRngType(fDefault)
{
  fpSteppingManager = new G4SteppingManager();
  messenger = new G4TrackingMessenger(this);
}

///////////////////////////////////////
G4TrackingManager::~G4TrackingManager()
///////////////////////////////////////
{
  delete messenger;
  delete fpSteppingManager;
  if (fpUserTrackingAction) delete fpUserTrackingAction;
}

////////////////////////////////////////////////////////////////
void G4TrackingManager::ProcessOneTrack(G4Track* apValueG4Track)
////////////////////////////////////////////////////////////////
{

  // Receiving a G4Track from the EventManager, this funciton has the
  // responsibility to trace the track till it stops.

  fpTrack = apValueG4Track;
  EventIsAborted = false;

  // Clear 2ndary particle vector
  //  GimmeSecondaries()->clearAndDestroy();    
  //  std::vector<G4Track*>::iterator itr;
  size_t itr;
  //  for(itr=GimmeSecondaries()->begin();itr=GimmeSecondaries()->end();itr++){ 
  for(itr=0;itr<GimmeSecondaries()->size();itr++){ 
     delete (*GimmeSecondaries())[itr];
  }
  GimmeSecondaries()->clear();  
   
  if(verboseLevel>0 && (G4VSteppingVerbose::GetSilent()!=1) ) TrackBanner();
  
  // Give SteppingManger the pointer to the track which will be tracked 
  fpSteppingManager->SetInitialStep(fpTrack);


  // this is the place to set the rng state from the newly received track
  auto const trackHash = (fRngType == fDefault) ? 0 : [this]()
  {
    auto* currentHepRandomEngine = G4Random::getTheEngine();
    auto hash = fpTrack->GetHash();
    if(!hash) // for the first track in the event
    {
      // use fixed width for bit operations
      uint32_t const low = static_cast<unsigned int>(*currentHepRandomEngine);
      uint32_t const high = static_cast<unsigned int>(*currentHepRandomEngine);
      int64_t const composition = (static_cast<int64_t>(high) << 32) | low;
      fpTrack->SetHash(composition);
      hash = composition;
    }
    auto const isMixMax = [currentHepRandomEngine]() -> G4bool
    {
      return  dynamic_cast<CLHEP::MixMaxRng*>(currentHepRandomEngine);
    };
    auto const isRanlux = [currentHepRandomEngine]() -> G4bool
    {
      return dynamic_cast<CLHEP::RanluxEngine*>(currentHepRandomEngine);
    };

    if((fRngType == fSpecial && isMixMax()) ||
       (isRanlux())) // because setSeeds sets an invalid state af Ranlux
    {
      if(debugOutput)
        G4cout << "G4TrackingManager::ProcessOneTrack: special seeding MixMax" << G4endl;
      constexpr uint64_t MASK32=0xffffffff;
      uint32_t const low = static_cast<uint64_t>(hash) & MASK32;
      uint32_t const high = static_cast<uint64_t>(hash) >> 32;
      std::array<G4long, 3> const seeds{{low, high, 0}};
      // for MixMaxRng setTheSeeds calls seed_uniquestream
      G4Random::setTheSeeds(seeds.data());
    }
    else
    {
      auto const isHepJames = [currentHepRandomEngine]() -> G4bool
      {
        return dynamic_cast<CLHEP::HepJamesRandom*>(currentHepRandomEngine);
      };
      if(isHepJames()) hash = static_cast<unsigned>(hash);
      G4Random::setTheSeed(hash);
    }
    return hash;
  }();

  if(debugOutput)
    G4cout << "start tracking hash " << trackHash << G4endl;

  // Pre tracking user intervention process.
  fpTrajectory = 0;
  if( fpUserTrackingAction != 0 ) {
     fpUserTrackingAction->PreUserTrackingAction(fpTrack);
  }
#ifdef G4_STORE_TRAJECTORY
  // Construct a trajectory if it is requested
  if(StoreTrajectory&&(!fpTrajectory)) { 
    // default trajectory concrete class object
    switch (StoreTrajectory) {
    default:
    case 1: fpTrajectory = new G4Trajectory(fpTrack); break;
    case 2: fpTrajectory = new G4SmoothTrajectory(fpTrack); break;
    case 3: fpTrajectory = new G4RichTrajectory(fpTrack); break;
    case 4: fpTrajectory = new G4RichTrajectory(fpTrack); break;
    }
  }
#endif

  // Give SteppingManger the maxmimum number of processes 
  fpSteppingManager->GetProcessNumber();

  // Give track the pointer to the Step
  fpTrack->SetStep(fpSteppingManager->GetStep());

  // Inform beginning of tracking to physics processes 
  fpTrack->GetDefinition()->GetProcessManager()->StartTracking(fpTrack);

  // Track the particle Step-by-Step while it is alive
  //  G4StepStatus stepStatus;

  while( (fpTrack->GetTrackStatus() == fAlive) ||
         (fpTrack->GetTrackStatus() == fStopButAlive) ){

    fpTrack->IncrementCurrentStepNumber();
    fpSteppingManager->Stepping();
#ifdef G4_STORE_TRAJECTORY
    if(StoreTrajectory) fpTrajectory->
                        AppendStep(fpSteppingManager->GetStep()); 
#endif
    if(EventIsAborted) {
      fpTrack->SetTrackStatus( fKillTrackAndSecondaries );
    }
  }
  // Inform end of tracking to physics processes 
  fpTrack->GetDefinition()->GetProcessManager()->EndTracking();

  // this is the place to assign rng state to the daughter tracks
  if(fRngType != fDefault)
  {
    auto* theSecondaries = GimmeSecondaries();
    auto const numOfSecondaries = theSecondaries->size();
    if(debugOutput)
    {
      if(numOfSecondaries) G4cout << "trackHash = " << trackHash << ", secondary seeds:";
    }
    for(auto ind = 0lu; ind < numOfSecondaries; ++ind)
    {
      auto* secondary = theSecondaries->at(ind);
      auto aHash = trackHash;
      auto aNewHash = std::hash<decltype(ind)>()(ind);
      hash_combine(aNewHash, aHash); // FIXME use a stronger hash
      auto seed = static_cast<signed>(aNewHash);
      secondary->SetHash(seed);
      if(debugOutput) G4cout << " " << seed;
    }
    if(debugOutput) if(numOfSecondaries) G4cout << G4endl;
  }

  // Post tracking user intervention process.
  if( fpUserTrackingAction != 0 ) {
     fpUserTrackingAction->PostUserTrackingAction(fpTrack);
  }

  // Destruct the trajectory if it was created
#ifdef G4VERBOSE
  if(StoreTrajectory&&verboseLevel>10) fpTrajectory->ShowTrajectory();
#endif
  if( (!StoreTrajectory)&&fpTrajectory ) {
      delete fpTrajectory;
      fpTrajectory = 0;
  }
}

void G4TrackingManager::SetTrajectory(G4VTrajectory* aTrajectory)
{
#ifndef G4_STORE_TRAJECTORY
  G4Exception("G4TrackingManager::SetTrajectory()",
              "Tracking0015", FatalException,
              "Invoked without G4_STORE_TRAJECTORY option set!");
#endif
  fpTrajectory = aTrajectory;
}

//////////////////////////////////////
void G4TrackingManager::EventAborted()
//////////////////////////////////////
{
  fpTrack->SetTrackStatus( fKillTrackAndSecondaries );
  EventIsAborted = true;
}


void G4TrackingManager::TrackBanner()
{
       G4cout << G4endl;
       G4cout << "*******************************************************"
            << "**************************************************"
            << G4endl;
       G4cout << "* G4Track Information: "
            << "  Particle = " << fpTrack->GetDefinition()->GetParticleName()
            << ","
            << "   Track ID = " << fpTrack->GetTrackID()
            << ","
            << "   Parent ID = " << fpTrack->GetParentID()
            << G4endl;
       G4cout << "*******************************************************"
            << "**************************************************"
            << G4endl;
       G4cout << G4endl;
}

namespace
{
//using boost::hash_combine now, TODO a choice including threefry, philox and crypto functions
template <class T> inline void hash_combine(std::size_t& seed, T const& v)
{
    seed ^= std::hash<T>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}
}






