// This code implementation is the intellectual property of
// the RD44 GEANT4 collaboration.
//
// By copying, distributing or modifying the Program (or any work
// based on the Program) you indicate your acceptance of this statement,
// and all its terms.
//
// $Id: G4CrossSectionDataStore.cc,v 2.0 1998/07/02 16:21:51 gunter Exp $
// GEANT4 tag $Name: geant4-00 $
//
//
// GEANT4 physics class: G4CrossSectionDataStore
// F.W. Jones, TRIUMF, 19-NOV-97
//

#include "G4CrossSectionDataStore.hh"


G4double
G4CrossSectionDataStore::GetCrossSection(const G4DynamicParticle* aParticle,
                                         const G4Element* anElement)
{
   if (NDataSetList == 0) {
      G4Exception("G4CrossSectionDataStore: no data sets registered");
      return DBL_MIN;
   }
   for (G4int i = NDataSetList-1; i >= 0; i--) {
      if (DataSetList[i]->IsApplicable(aParticle, anElement))
             return DataSetList[i]->GetCrossSection(aParticle, anElement);
   }
   G4Exception("G4CrossSectionDataStore: no applicable data set found "
               "for particle/element");
   return DBL_MIN;
}


void
G4CrossSectionDataStore::AddDataSet(G4VCrossSectionDataSet* aDataSet)
{
   if (NDataSetList == NDataSetMax) {
      G4Exception("G4CrossSectionDataStore::AddDataSet: "
                  "reached maximum number of data sets");
      return;
   }
   DataSetList[NDataSetList] = aDataSet;
   NDataSetList++;
}


void
G4CrossSectionDataStore::
BuildPhysicsTable(const G4ParticleDefinition& aParticleType)
{
   if (NDataSetList == 0) {
      G4Exception("G4CrossSectionDataStore: no data sets registered");
      return;
   }
   for (G4int i = NDataSetList-1; i >= 0; i--) {
      DataSetList[i]->BuildPhysicsTable(aParticleType);
   }
}


void
G4CrossSectionDataStore::
DumpPhysicsTable(const G4ParticleDefinition& aParticleType)
{
   if (NDataSetList == 0) {
      G4Exception("G4CrossSectionDataStore: no data sets registered");
      return;
   }
   for (G4int i = NDataSetList-1; i >= 0; i--) {
      DataSetList[i]->DumpPhysicsTable(aParticleType);
   }
}