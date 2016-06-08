// This code implementation is the intellectual property of
// the RD44 GEANT4 collaboration.
//
// By copying, distributing or modifying the Program (or any work
// based on the Program) you indicate your acceptance of this statement,
// and all its terms.
//
// $Id: G4ProcessTable.cc,v 2.5 1998/09/14 12:02:57 kurasige Exp $
// GEANT4 tag $Name: geant4-00 $
//
// 
// ------------------------------------------------------------
//	GEANT 4 class header file 
//
//	For information related to this code contact:
//	CERN, IT Division, ASD group
//	History: first implementation, based on object model of
//	4th Aug 1998, H.Kurashige
// ------------------------------------------------------------

#include "G4ProcessTableMessenger.hh"
#include "G4ProcessTable.hh"

//  constructor //////////////////////////
G4ProcessTable::G4ProcessTable():verboseLevel(1)
{
#ifdef G4VERBOSE
  if (verboseLevel>1){
    G4cerr << "--  G4ProcessTable constructor  --" << endl;
  }
#endif
  fProcTblVector  = new  G4ProcTableVector();
  fProcNameVector = new  G4ProcNameVector();
  tmpTblVector    = new  G4ProcTableVector();
  fProcTblMessenger = NULL;
}

// copy constructor //////////////////////////
G4ProcessTable::G4ProcessTable(const G4ProcessTable &right)
{
#ifdef G4VERBOSE
  if (verboseLevel>0){
    G4cerr << "--  G4ProcessTable copy constructor  --" << endl;
  }
#endif
}

// destructor //////////////////////////
G4ProcessTable::~G4ProcessTable()
{
#ifdef G4VERBOSE
  if (verboseLevel>1){
    G4cerr << "--  G4ProcessTable destructor  --" << endl;
  }

#endif
  // delete all processes
  for (G4int idxTbl =0; idxTbl <fProcTblVector->length(); idxTbl++){
    delete ((*fProcTblVector)(idxTbl))->GetProcess();
  }
 
  if ( tmpTblVector != NULL) {
    tmpTblVector ->clear();
    delete tmpTblVector;
  }
  if ( fProcTblVector != NULL) {
    fProcTblVector ->clearAndDestroy();
    delete fProcTblVector;
  }
  if ( fProcNameVector != NULL) {
    fProcNameVector ->clear();
    delete fProcNameVector;
  }
}

/////////////////////////
G4UImessenger* G4ProcessTable::CreateMessenger()
{
  if (fProcTblMessenger == NULL) {
    fProcTblMessenger = new G4ProcessTableMessenger(this);
  }
  return     fProcTblMessenger;
}

/////////////////////////
void  G4ProcessTable::DeleteMessenger()
{
  if (fProcTblMessenger != NULL) {
    delete fProcTblMessenger;
  }
}


//////////////////////////
G4ProcessTable & G4ProcessTable::operator=(const G4ProcessTable &right)
{
#ifdef G4VERBOSE
  if (verboseLevel>0){
    G4cerr << "--  G4ProcessTable assignment operator  --" << endl;
  }
#endif
  return *this;
}

//////////////////////////
G4int G4ProcessTable::operator==(const G4ProcessTable &right) const
{
  return (this == &right);
}

//////////////////////////
G4int G4ProcessTable::operator!=(const G4ProcessTable &right) const
{
  return (this != &right);
}

// Static class variable: ptr to single instance of class
G4ProcessTable* G4ProcessTable::fProcessTable =0;


//////////////////////////
G4ProcessTable* G4ProcessTable::GetProcessTable()
{
    static G4ProcessTable theProcessTable;
    if (!fProcessTable){
      fProcessTable =  &theProcessTable;
    }
    return fProcessTable;
}

//////////////////////////
G4int   G4ProcessTable::Insert(G4VProcess* aProcess, 
			       G4ProcessManager* aProcMgr)
{
  if ( (aProcess == NULL) || ( aProcMgr == NULL ) ){
#ifdef G4VERBOSE
    if (verboseLevel>0){
      G4cerr << "G4ProcessTable::Insert : arguments are NULL pointer "<<endl;
    }
#endif
    return -1;
  }
    
#ifdef G4VERBOSE
  if (verboseLevel>1){
    G4cerr << "G4ProcessTable::Insert ";
    G4cerr << " Process["  << aProcess->GetProcessName() << "]";
    G4cerr << " Particle["  << aProcMgr->GetParticleType()->GetParticleName() << "]";
    G4cerr << endl;
  }
#endif

  G4int idxTbl;
  G4ProcTblElement* anElement;
  G4bool isFoundInTbl = false;
  // loop over all elements
  for (idxTbl =0; idxTbl <fProcTblVector->length(); idxTbl++){
    anElement = (*fProcTblVector)(idxTbl);
    G4int idx;
    // check if this process is included
    if (aProcess == anElement->GetProcess()) {
      isFoundInTbl = true;
#ifdef G4VERBOSE
      if (verboseLevel>2){
	G4cerr << " This Process has been already registered !! ";
	G4cerr << endl;
      }
#endif
      // 
      G4bool isFound = false;
      // check if this process manager is included
      for (idx=0; idx<anElement->Length(); idx++){
	if ( aProcMgr  == anElement->GetProcessManager(idx) ){
	  isFound = true;
#ifdef G4VERBOSE
	  if (verboseLevel>1){
	    G4cerr << " This Process Manager has been already registered !! ";
	    G4cerr << endl;
	  }
#endif
	  break;
	}
      }
      // add the process manager into the element
      if (!isFound) {
	anElement->Insert(aProcMgr);
#ifdef G4VERBOSE
	if (verboseLevel>2){
	  G4cerr << " This Process Manager is registered !! ";
	  G4cerr << endl;
	}
#endif
      }
      break;
    }
  }
  // add this process into the table
  if (!isFoundInTbl) {
    G4ProcTblElement* newElement = new G4ProcTblElement(aProcess);
    newElement->Insert(aProcMgr);
    fProcTblVector->insert(newElement);
    // add into name vector     
    if (!fProcNameVector->contains(aProcess->GetProcessName()) ){
      fProcNameVector->insert(aProcess->GetProcessName() );
    }
#ifdef G4VERBOSE
    if (verboseLevel>2){
      G4cerr << " This Process is registered !! ";
      G4cerr << endl;
    }
#endif
  }
  return idxTbl;
}

//////////////////////////
G4int  G4ProcessTable::Remove( G4VProcess* aProcess, 
			       G4ProcessManager* aProcMgr)
{
  if ( (aProcess == NULL) || ( aProcMgr == NULL ) ){
#ifdef G4VERBOSE
    if (verboseLevel>0){
      G4cerr << "G4ProcessTable::Remove : arguments are NULL pointer "<<endl;
    }
#endif
    return -1;
  }
    
#ifdef G4VERBOSE
  if (verboseLevel>1){
    G4cerr << "G4ProcessTable::Remove ";
    G4cerr << " Process["  << aProcess->GetProcessName() << "]";
    G4cerr << " Particle["  << aProcMgr->GetParticleType()->GetParticleName() << "]";
    G4cerr << endl;
  }
#endif

  G4int idxTbl;
  G4ProcTblElement* anElement;
  G4bool isFoundInTbl = false;
  G4bool isFound = false;
  // loop over all elements
  for (idxTbl =0; idxTbl <fProcTblVector->length(); idxTbl++){
    anElement = (*fProcTblVector)(idxTbl);
    G4int idx;
    // check if this process is included
    if (aProcess == anElement->GetProcess()) {
      isFoundInTbl = true;

      // check if this process manager is included
      for (idx=0; idx<anElement->Length(); idx++){
	if ( aProcMgr  == anElement->GetProcessManager(idx) ){
	  isFound = true;
	  break;
	}
      }
      // add the process manager into the element
      if (isFound) {
	anElement->Remove(aProcMgr);
#ifdef G4VERBOSE
	if (verboseLevel>2){
	  G4cerr << " This Process Manager is removed !! ";
	  G4cerr << endl;
	}
#endif
      }
      break;
    }
  }
  if (isFound) {
    if (anElement->Length() == 0){
      fProcTblVector->remove(anElement);
      delete anElement;
      // check other prcesses with same name exist or not
      G4bool isSameName = false;
      for (idxTbl =0; idxTbl <fProcTblVector->length(); idxTbl++){
	anElement = (*fProcTblVector)(idxTbl);
        if (anElement->GetProcessName() == aProcess->GetProcessName()) {
	  isSameName = true;
	  break;
	}
      }
      // remove from name vector
      if (!isSameName ) {
	fProcNameVector->remove(aProcess->GetProcessName() );
      }
#ifdef G4VERBOSE
      if (verboseLevel>1){
	G4cerr << " This Process is removed !! ";
	G4cerr << endl;
      }
#endif
    }
    return idxTbl;
  } else {
#ifdef G4VERBOSE
    if (verboseLevel>0){
      G4cerr << " This Process is not registered !! ";
      G4cerr << endl;
    }
#endif
  }
  return -1;
}

//////////////////////////
G4VProcess* G4ProcessTable::FindProcess(const G4String& processName, 
			                const G4ProcessManager* processManager)
                                        const
{
  G4int idxTblVec;
  G4bool isFound = false;
  G4ProcTblElement* anElement;
  for ( idxTblVec = 0;  idxTblVec < fProcTblVector->length(); idxTblVec++){
    anElement = (*fProcTblVector)(idxTblVec);
    // check name
    if ( anElement->GetProcessName() == processName ) {
      // check if the processManage is included
      if ( anElement->Contains(processManager) ) {
	isFound = true;
	break;
      }
    }
  }
#ifdef G4VERBOSE
  if (!isFound && verboseLevel>1){
    G4cerr << " G4ProcessTable::FindProcess :" ;
    G4cerr << " The Process[" << processName << "] is not found  ";
    G4cerr << " for " << processManager->GetParticleType()->GetParticleName();
    G4cerr << endl;
  }
#endif
  
  if (isFound) return anElement->GetProcess();
  else         return NULL;
}

///////////////
G4ProcessTable::G4ProcTableVector* G4ProcessTable::Find( 
					 G4ProcTableVector* procTblVector,
					 const G4String& processName )
{
  tmpTblVector-> clear();

  G4int idxTblVec;
  G4bool isFound = false;
  G4ProcTblElement* anElement;
  for ( idxTblVec = 0;  idxTblVec < procTblVector->length(); idxTblVec++){
    anElement = (*procTblVector)(idxTblVec);
    // check name
    if ( anElement->GetProcessName() == processName ) {
      isFound = true;
      tmpTblVector->insert(anElement);
    }
  }

#ifdef G4VERBOSE
  if (!isFound && verboseLevel>0){
    G4cerr << " G4ProcessTable::Find :" ;
    G4cerr << " The Process[" << processName << "] is not found  ";
    G4cerr << endl;
  }
#endif
  
  return tmpTblVector;

}     
///////////////
G4ProcessTable::G4ProcTableVector* G4ProcessTable::Find( 
					 G4ProcTableVector* procTblVector,
					 G4ProcessType   processType )
{
  tmpTblVector->clear();

  G4int idxTblVec;
  G4bool isFound = false;
  G4ProcTblElement* anElement;
  for ( idxTblVec = 0;  idxTblVec < procTblVector->length(); idxTblVec++){
    anElement = (*procTblVector)(idxTblVec);
    // check name
    if ( anElement->GetProcess()->GetProcessType() == processType ) {
      isFound = true;
      tmpTblVector->insert(anElement);
    }
  }

#ifdef G4VERBOSE
  if (!isFound && verboseLevel>0){
    G4cerr << " G4ProcessTable::Find :" ;
    G4cerr << " The Process[" << anElement->GetProcessName() << "] is not found  ";
    G4cerr << endl;
  }
#endif
  
  return tmpTblVector;

}     

///////////////
G4ProcessVector* G4ProcessTable::ExtractProcesses( G4ProcTableVector* procTblVector)
{
  G4ProcessVector* procList = new G4ProcessVector();
  G4int idxTblVec;
  for ( idxTblVec = 0;  idxTblVec < procTblVector->length(); idxTblVec++){
    G4ProcTblElement* anElement = (*procTblVector)(idxTblVec);
    procList->insert( anElement->GetProcess() );
  }
  return procList;
}

///////////////
G4ProcessVector* G4ProcessTable::FindProcesses()
{
  return ExtractProcesses(fProcTblVector);
}

///////////////
G4ProcessVector* G4ProcessTable::FindProcesses( const G4ProcessManager* pManager )
{
  G4ProcessVector* procList = pManager->GetProcessList();
  return new G4ProcessVector(*procList);
}

///////////////
G4ProcessVector* G4ProcessTable::FindProcesses( const G4String& processName )
{
  G4ProcTableVector* pTblVector =  Find(fProcTblVector, processName);
  return ExtractProcesses(pTblVector);
}

///////////////
G4ProcessVector* G4ProcessTable::FindProcesses( G4ProcessType processType)
{
  G4ProcTableVector* pTblVector =  Find(fProcTblVector, processType);
  return ExtractProcesses(pTblVector);
}

///////////////
void G4ProcessTable::SetProcessActivation( const G4String& processName, 
			                   G4bool          fActive  )
{
#ifdef G4VERBOSE
  if (verboseLevel>1){
    G4cerr << " G4ProcessTable::SetProcessActivation:" ;
    G4cerr << " The Process[" << processName << "] "<< endl;
  }
#endif

  G4ProcTableVector* pTblVector =  Find(fProcTblVector, processName);
  G4int idxTblVec;
  G4ProcTblElement* anElement;  
  for ( idxTblVec = 0;  idxTblVec < pTblVector->length(); idxTblVec++){
    anElement = (*pTblVector)(idxTblVec);
    G4VProcess* process = anElement->GetProcess();
    for (G4int idx = 0 ; idx < anElement->Length(); idx++) {
      G4ProcessManager* manager = anElement->GetProcessManager(idx);
      manager->SetProcessActivation(process, fActive);
#ifdef G4VERBOSE
      if (verboseLevel>1){
        G4cerr << "  for " << manager->GetParticleType()->GetParticleName();
	G4cerr << "  Index = " << manager->GetProcessIndex(process); 
        G4cerr << endl;
      }
#endif
    }
  }
}

///////////////
void G4ProcessTable::SetProcessActivation( 
			   const G4String& processName, 
		           G4ProcessManager* processManager, 
			   G4bool          fActive  )
{
#ifdef G4VERBOSE
  if (verboseLevel>1){
    G4cerr << " G4ProcessTable::SetProcessActivation:" ;
    G4cerr << " The Process[" << processName << "] "<< endl;
  }
#endif
  
  G4VProcess* process = FindProcess( processName,  processManager);
  if ( process != NULL) {
    processManager->SetProcessActivation(process, fActive);
#ifdef G4VERBOSE
    if (verboseLevel>1){
      G4cerr << "  for " << processManager->GetParticleType()->GetParticleName();
      G4cerr << "  Index = " << processManager->GetProcessIndex(process); 
      G4cerr << endl;
    }
#endif
  } 
}


///////////////
void G4ProcessTable::SetProcessActivation( G4ProcessType   processType, 
			                   G4bool          fActive  )
{
#ifdef G4VERBOSE
  if (verboseLevel>1){
    G4cerr << " G4ProcessTable::SetProcessActivation:" ;
    G4cerr << " The ProcessType[" << processType << "] "<< endl;
  }
#endif

  G4ProcTableVector* pTblVector =  Find(fProcTblVector, processType);
  G4int idxTblVec;
  G4ProcTblElement* anElement;  
  for ( idxTblVec = 0;  idxTblVec < pTblVector->length(); idxTblVec++){
    anElement = (*pTblVector)(idxTblVec);
    G4VProcess* process = anElement->GetProcess();
#ifdef G4VERBOSE
    if (verboseLevel>1){
      G4cerr << " The Process[" << process->GetProcessName()<< "] "<< endl;
    }
#endif
    for (G4int idx = 0 ; idx < anElement->Length(); idx++) {
      G4ProcessManager* manager = anElement->GetProcessManager(idx);
      manager->SetProcessActivation(process, fActive);
#ifdef G4VERBOSE
      if (verboseLevel>1){
        G4cerr << "  for " << manager->GetParticleType()->GetParticleName();
	G4cerr << "  Index = " << manager->GetProcessIndex(process); 
        G4cerr << endl;
      }
#endif
    }
  }
}

///////////////
void G4ProcessTable::SetProcessActivation( 
			   G4ProcessType   processType, 
		           G4ProcessManager* processManager, 
			   G4bool          fActive  )
{
#ifdef G4VERBOSE
  if (verboseLevel>1){
    G4cerr << " G4ProcessTable::SetProcessActivation:" ;
    G4cerr << " The ProcessType[" << processType << "] "<< endl;
  }
#endif
  
  G4ProcessVector* procList =  processManager->GetProcessList();
  for (G4int idx = 0; idx < procList->length(); idx++) {
    G4VProcess* process = (*procList)(idx);
    if ( process->GetProcessType() == processType) {
      processManager->SetProcessActivation(process, fActive);
#ifdef G4VERBOSE
      if (verboseLevel>1){
        G4cerr << " The Process[" << process->GetProcessName()<< "] "<< endl;
	G4cerr << "  for " << processManager->GetParticleType()->GetParticleName();
	G4cerr << "  Index = " << idx; 
	G4cerr << endl;
      }
#endif
    }
  }
}


/////////////
void G4ProcessTable::DumpInfo(G4VProcess* process, 
			      G4ParticleDefinition* particle)
{
  G4int idxTbl;
  G4ProcTblElement* anElement;
  G4bool isFoundInTbl = false;
  G4ProcessManager* manager;
  G4int idx;
  // loop over all elements
  for (idxTbl =0; idxTbl <fProcTblVector->length(); idxTbl++){
    anElement = (*fProcTblVector)(idxTbl);
    if (process == anElement->GetProcess() ){
      if (particle==NULL) {
	isFoundInTbl = true;
	break;
      } else {
	for (idx=0; idx<anElement->Length(); idx++){
	  manager = anElement->GetProcessManager(idx);
	  if (particle == manager->GetParticleType()) {
	    isFoundInTbl = true;
	    break;
	  }
	}
      }
    }
  }
  if  (!isFoundInTbl ) return;
  
  G4int tmpVerbose = process->GetVerboseLevel();
  process->SetVerboseLevel(verboseLevel);
  process->DumpInfo();
  process->SetVerboseLevel(tmpVerbose);
  if (particle==NULL) {
    for (idx=0; idx<anElement->Length(); idx++){
      manager = anElement->GetProcessManager(idx);
      G4cout << " for " << manager->GetParticleType()->GetParticleName();
      G4cout << endl;
      if (verboseLevel >2) manager->DumpInfo();
    }
  } else {
    G4cout << " for " << manager->GetParticleType()->GetParticleName();
    G4cout << endl;
    if (verboseLevel >2) manager->DumpInfo();
  }
}





