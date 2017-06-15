//    Copyright (c) 2017 Dmitry Savin <sd57@protonmail.ch>
//
//    Distributed under the GNU General Public License version 3.

#include "G4Cbprng.hh"
#include "uniform.hpp"
#include "CLHEP/Random/engineIDulong.h"
#include "ExtendedEngineFactory.hh"
#include "CLHEP/Utility/atomic_int.h"
#include <type_traits>

#include "random123.hpp"

// uncomment for debugging output
//#define G4CBPRNGDEBUG

namespace CLHEP
{

namespace {
  // Number of instances with automatic seed selection
  CLHEP_ATOMIC_INT_TYPE numberOfEngines(0);
}

// to be made private
template<typename RNG_t>
G4Cbprng<RNG_t>::G4Cbprng(ctr_type const& aCtr, key_type const& aKey)
  : HepRandomEngine(), fCtr(aCtr), fKey(aKey)
{
  numberOfEngines++;
}

template<typename RNG_t>
G4Cbprng<RNG_t>::G4Cbprng(): G4Cbprng(ctr_type{{}})
{
  int const numEngines = numberOfEngines++;
  setSeed(static_cast<long>(numEngines));
}

template<typename RNG_t>
double G4Cbprng<RNG_t>::flat()
{
  // FIXME assumes that randVals is an array of two 32-bit integers
  // may work for bigger outputs
  // TODO make a function to use in flatArray
  auto const randVals = fRNG(fCtr, fKey);
  uint32_t const low = randVals.front();
  uint32_t const high = randVals.back();
  uint64_t composition = static_cast<uint64_t>(high) << 32 | low;
  double const value = r123::u01<double>(composition);
  ++fKey.front();
  return value;
}

template<typename RNG_t>
void G4Cbprng<RNG_t>::flatArray(const int size, double* vect)
{
#warning "Untested"
//  TODO make vectorizable
  if(size <= 0) return;
  std::vector<ret_type> results;
  for(auto it = 0; it < size; ++it)
  {
    results.push_back(fRNG(fCtr, fKey));
    ++fKey.front();
  }
  // TODO use std::transform
  for(auto it = 0; it < size; ++it)
  {
    auto const& res = results.at(static_cast<unsigned>(it));
    uint32_t const low = res.front();
    uint32_t const high = res.back();
    uint64_t composition = static_cast<uint64_t>(high) << 32 | low;
    double const value = r123::u01<double>(composition);
    vect[it] = value;
  }
}

template<typename RNG_t>
void G4Cbprng<RNG_t>::setSeed (long aCtr, int index)
{
  // int and long widths are imlementation specific
  // TODO indicate that only 32 bits are used
  size_t const position = static_cast<size_t>(index)%fCtr.size();
  fCtr.at(position) = static_cast<typename ctr_type::value_type>(aCtr);
}

template<typename RNG_t>
void G4Cbprng<RNG_t>::setSeeds (const long* seeds, int aKey)
{
  // int and long widths are imlementation specific
  // TODO indicate that only 32 bits are used
  fCtr.front() = static_cast<typename ctr_type::value_type>(seeds[0]);
  fCtr.back() = static_cast<typename ctr_type::value_type>(seeds[1]);
  fKey = {{static_cast<typename key_type::value_type>(aKey)}};
}

template<typename RNG_t>
void G4Cbprng<RNG_t>::saveStatus( const char filename[]) const
{
  std::ofstream outFile(filename, std::ios::out);
  if (outFile.good())
  {
    outFile << fCtr << ' ';
    outFile << fKey;
  }
  else
  {
    std::cerr << "G4Cbprng::restoreStatus(): Engine state remains unsaved\n";
  }
  outFile.close();
}

template<typename RNG_t>
void G4Cbprng<RNG_t>::restoreStatus( const char filename[])
{
  std::ifstream inFile(filename, std::ios::in);
  if (!checkFile ( inFile, filename, name(), "restoreStatus" ))
  {
    std::cerr << "G4Cbprng::restoreStatus(): Engine state remains unchanged\n";
    return;
  }
  for(auto& it: fCtr)
  {
    inFile >> it;
  }
  for(auto& it: fKey)
  {
    inFile >> it;
  }
}

template<typename RNG_t>
std::vector<unsigned long> G4Cbprng<RNG_t>::put () const
{
  std::vector<unsigned long> aVector;
  aVector.push_back(engineIDulong<G4Cbprng<RNG_t>>());
  aVector.insert(aVector.end(), fCtr.begin(), fCtr.end());
  aVector.insert(aVector.end(), fKey.begin(), fKey.end());
  return aVector;
}

template<typename RNG_t>
bool G4Cbprng<RNG_t>::get (const std::vector<unsigned long> & v)
{
  if ((v[0] & 0xffffffffUL) != engineIDulong<G4Cbprng<RNG_t>>())
  {
    std::cerr <<
                 "\nG4Cbprng get:state vector has wrong ID word - state unchanged\n";
    return false;
  }
  return getState(v);
}

template<typename RNG_t>
bool G4Cbprng<RNG_t>::getState (const std::vector<unsigned long> & v)
{
  if (v.size() != VECTOR_STATE_SIZE ) {
    std::cerr << "\nG4Cbprng get:state vector has wrong length - state unchanged\n";
#ifdef G4CBPRNGDEBUG
    std::cerr << "v.size() = " << v.size()
              << ", VECTOR_STATE_SIZE = " << VECTOR_STATE_SIZE
              << ", ctr_type::static_size = " << ctr_type::static_size
              << ", key_type::static_size = " << key_type::static_size
              << ", v =";
    for(auto it: v) std::cerr << " " << it;
    std::cerr << std::endl;
#endif
    return false;
  }
  std::copy_n(v.begin() + 1, ctr_type::static_size, fCtr.begin());
  std::copy_n(v.begin() + 1 + ctr_type::static_size, key_type::static_size, fKey.begin());
  return true;
}

template<typename RNG_t>
void G4Cbprng<RNG_t>::showStatus() const
{
  std::cout << "G4Cbprng::showStatus(): "
            << "fCtr = " << fCtr
            << ", fKey = " << fKey
            << std::endl;
}

template<typename RNG_t>
HepRandomEngine* G4Cbprng<RNG_t>::newEngine(const std::vector<unsigned long> & v)
{
  return ExtendedEngineFactory::newEngine(v);
}

template<typename RNG_t>
std::string G4Cbprng<RNG_t>::beginTag ( )
{
  return engineName() + "-begin";
}

template<>
std::string G4Cbprng<r123::Philox2x32>::engineName()
{
    return "CbprngPhilox2x32";
}

template<>
std::string G4Cbprng<r123::Threefry2x32>::engineName()
{
    return "CbprngThreefry2x32";
}

template class G4Cbprng<r123::Philox2x32>;
template class G4Cbprng<r123::Threefry2x32>;

} // namespace CLHEP

