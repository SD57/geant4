//    Copyright (c) 2017 Dmitry Savin <sd57@protonmail.ch>
//
//    Distributed under the GNU General Public License version 3.
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
  G4Cbprng(ctr_type const& aCtr, key_type const& aKey = {{0x0}});
  G4Cbprng();

  virtual double flat() override;
  // Returns a pseudo random number between 0 and 1
  // (excluding the end points)

  virtual void flatArray(const int size, double* vect) override;
  // Fills an array "vect" of specified size with flat random values.

  virtual void setSeed (long aCtr, int index = 0) override;
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

  static std::string engineName() {return "Cbprng";}
  // Static engine name, must be defined explicitly for the instances

  virtual std::string name() const override {return engineName();}
  // Engine name.
  static std::string beginTag();

private:
  RNG fRNG;
  ctr_type fCtr;
  key_type fKey;

  static const unsigned int VECTOR_STATE_SIZE = 1 + ctr_type::static_size
                                                  + key_type::static_size;

};

} // namespace CLHEP
#endif // G4CBPRNG_H
