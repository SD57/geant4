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

/// HepRandomEngine interface class for a counter-based PRNG from Random123 library

#ifndef G4CBPRNG_H
#define G4CBPRNG_H

#include "CLHEP/Random/RandomEngine.h"

namespace CLHEP
{
 // FIXME now the implementation depends on the default type
template<typename RNG_t>
class G4Cbprng: public HepRandomEngine
{
public:
  using RNG = RNG_t;
  using ctr_type = typename RNG::ctr_type;
  using key_type = typename RNG::key_type;
  using ret_type = ctr_type;
  G4Cbprng(ctr_type const& aCtr, key_type const& aKey = {{}});
  G4Cbprng();
  virtual ~G4Cbprng() override;

  virtual double flat() override;
  // Returns a pseudo random number between 0 and 1
  // (excluding the end points)

  virtual void flatArray(const int size, double* vect) override;
  // Fills an array "vect" of specified size with flat random values.

  virtual void setSeed (long aCtr, int = 0) override;
  // Resets the state of the algorithm according to "index", the position
  // in the static table of seeds stored in HepRandom.

  virtual void setSeeds (const long * seeds, int) override;
  // Sets the state of the algorithm according to the array of seeds and int as the key
  // "seeds" containing two seed values to be stored as the seeds

  virtual void saveStatus( const char filename[] = "Cbprng.conf" ) const override;
  // Saves on file Cbprng.conf the current engine status.

  virtual void restoreStatus( const char filename[] = "Cbprng.conf" ) override;
  // Reads from file Cbprng.conf the last saved engine status
  // and restores it.

  virtual void showStatus() const override;
  // Dumps the engine status on the screen.

  virtual std::vector<unsigned long> put () const override;
  virtual bool get (const std::vector<unsigned long> & v) override;
  virtual bool getState (const std::vector<unsigned long> & v) override;
  // Save and restore to/from vectors

  virtual std::ostream & put (std::ostream & os) const override;
  virtual std::istream & get (std::istream & is) override;
  // Save and restore to/from streams

  virtual std::istream & getState ( std::istream & is ) override;
  // Helpers for EngineFactory which restores anonymous engine from istream

  static std::string engineName();
  // Static engine name, must be defined explicitly for the instances

  virtual std::string name() const override;
  // Engine name.
  static std::string beginTag();

private:
  RNG fRNG;
  ctr_type fCtr;
  key_type fKey;

  void increaseKey();
  void increaseCtr();

  static const unsigned int VECTOR_STATE_SIZE = 1 + ctr_type::static_size
                                                  + key_type::static_size;

};

} // namespace CLHEP
#endif // G4CBPRNG_H
