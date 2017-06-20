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

#include "CLHEP/Random/RandomEngine.h" // FIXME for the clang-based IDE parser, redundant
#include "CLHEP/Random/G4Cbprng.h"
#include "CLHEP/Random/random123.h"

#if defined(G4MULTITHREADED)
#define HEPRNDM G4MTHepRandom
#else
#define HEPRNDM CLHEP::HepRandom
#endif

namespace
{
template <class T> void hash_combine(std::size_t& seed, T const& v);
 // TODO check that it is a container of hashable types
template <class Container> std::size_t hashContainer(Container const& aContainer,
                                                     std::size_t aSeed = 0);
}


class G4VSteppingVerbose;

//////////////////////////////////////
G4TrackingManager::G4TrackingManager()
//////////////////////////////////////
  : fpUserTrackingAction(0), fpTrajectory(0),
    StoreTrajectory(0), verboseLevel(0), EventIsAborted(false)
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

  // TODO this is the place to set the rng state from the newly received track
  auto const& trackRNGstate = fpTrack->GetRNGstate();
  if(trackRNGstate.size()) // safety to work before fully implemented
  {
    CLHEP::HepRandomEngine* currentEngine = HEPRNDM::getTheEngine();
    currentEngine->get(trackRNGstate);
  }
  else
  {
    CLHEP::HepRandomEngine* currentEngine = HEPRNDM::getTheEngine();
    fpTrack->SetRNGstate(currentEngine->put());
  }

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

  // TODO this is the place to assign rng state to the daughter tracks
  CLHEP::HepRandomEngine& currentEngine = *HEPRNDM::getTheEngine();
  auto* theSecondaries = GimmeSecondaries();
  auto const numOfSecondaries = theSecondaries->size();
  for(auto ind = 0u; ind < numOfSecondaries; ++ind)
  {
    auto* secondary = theSecondaries->at(ind);
    uint32_t const low = static_cast<unsigned int>(currentEngine);//fixed width for bit operations
    uint32_t const high = static_cast<unsigned int>(currentEngine);
    uint64_t const composition = static_cast<uint64_t>(high) << 32 | low;
    auto seed = static_cast<signed>(hashContainer(trackRNGstate, composition));
    currentEngine.setSeed(seed, 0); // FIXME: is the second parameter used by any engine?
    auto newState = currentEngine.put();
    secondary->SetRNGstate(newState);
  }

  // TODO chea\ck if it is a counter based
  // then iterate over all secondaries incrementing the major counter
  //    G4bool isCBPRNG = dynamic_cast<CLHEP::G4Cbprng<r123::Philox2x32>*>(currentEngine);
  //    if(isCBPRNG)
  //    {
  //      auto* theCBPRNGEngine = static_cast<CLHEP::G4Cbprng<r123::Philox2x32>*>(currentEngine);

  //      currentEngine->get(trackRNGstate);
  //    }
  //    else
  //    {

  //    }


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

template <class Container> inline std::size_t hashContainer(Container const& aContainer,
                                                            std::size_t aSeed)
{
  for(auto const& it : aContainer)
  {
    hash_combine(aSeed, it);
  }
  return aSeed;
}
}






