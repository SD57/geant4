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

/// Header to include Random123 files and suppress repetative warnings

#ifndef RANDOM123_H
#define RANDOM123_H

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexpansion-to-defined"
#endif
#include "CLHEP/Random/include/philox.h"
#include "CLHEP/Random/include/threefry.h"
#include "CLHEP/Random/include/ReinterpretCtr.hpp"
#include "CLHEP/Random/examples/uniform.hpp"
#ifdef __clang__
#ifndef __clangfeatures_dot_hpp
#warning "Disabling -Wexpansion-to-defined for clean compile output"
#endif
#pragma clang diagnostic pop
// FIXME looks like the diagnostic still remains suppressed
#endif

#endif // RANDOM123_H
