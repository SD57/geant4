// This code implementation is the intellectual property of
// the RD44 GEANT4 collaboration.
//
// By copying, distributing or modifying the Program (or any work
// based on the Program) you indicate your acceptance of this statement,
// and all its terms.
//
// $Id: G4StackedTrack.cc,v 2.0 1998/07/02 16:54:06 gunter Exp $
// GEANT4 tag $Name: geant4-00 $
//
//
//  Last Modification : 02/Feb/96 M.Asai
//

#include "G4StackedTrack.hh"

G4Allocator<G4StackedTrack> aStackedTrackAllocator;

G4StackedTrack::G4StackedTrack() 
{ }

G4StackedTrack::G4StackedTrack(G4Track * newTrack) 
:track(newTrack),priorityWeight(0.),
 previousStackedTrack(NULL),nextStackedTrack(NULL)
{ }

G4StackedTrack::~G4StackedTrack()
{ }

const G4StackedTrack & G4StackedTrack::operator=(const G4StackedTrack &right)
{ return *this; }
int G4StackedTrack::operator==(const G4StackedTrack &right) const 
{ return false; }
int G4StackedTrack::operator!=(const G4StackedTrack &right) const 
{ return true; }

