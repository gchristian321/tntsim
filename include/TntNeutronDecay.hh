/// Defines classes for generating neutron decays of various types
#ifndef TNT_NEUTRON_DECAY_HH
#define TNT_NEUTRON_DECAY_HH
#include <map>
#include <G4String.hh>
#include <G4ThreeVector.hh>
#include <G4LorentzVector.hh>

class TntRng;
class TntParticle;

/// Abstract base class for generic neutron decay generators
class TntNeutronDecay {
public:
  /// Ctor
	TntNeutronDecay() { }
	/// Dtor
	virtual ~TntNeutronDecay() { }
	/// Set input particle (in excited unbound state)
	/** Parameters for initial decaying nucleus are taken from
	 *  the input of this function.
	 *  \attention It shold be called every time, before calling Generate()
	 */
	virtual void SetInputParticle(const TntParticle* p) = 0;
	/// Set pointer to RNG used for excitation energy generation
	/** \todo Make this something that is created in the class itself
	 * and then returned! To allow for polymorphism automatically...
	 */
	virtual void SetRngEx(const TntRng* rng) = 0;
	/// Set verbosity level
	/** 0: Print nothing
	 *  1: Print fatal errror/warning messages only (to G4cerr)
	 *  2: Print all error/warning messages (to G4cerr)
	 */
	virtual void SetVerboseLevel(G4int level) = 0;
	/// Returns vebosity level
	virtual G4int GetVerboseLevel() const = 0;
	/// Returns number of neutrons emitted in the decay
	virtual G4int GetNumberOfNeutrons() const = 0;
	/// Set some generic decay parameter (id'd by string)
	virtual void SetDecayParam(const G4String& par, G4double val) = 0;
	/// Return some generic decay paramter (id'd by string)
	virtual G4double GetDecayParam(const G4String& par) = 0;
	/// Do the actual neutron decay generation
	/** Needs to be implemented separately for every decay type.
	 *  \returns `true` if decay was successful, `false` if there
	 *            wasn't enough energy in the initial state 
	 *            for the decay to happen
	 */
	virtual G4bool Generate() = 0;
	/// Return the final states, after calling Generate()
	/** \param indx 0: initial state; 1: final fragment; 2,3,...: neutrons 0,1,...
	 */
	virtual const G4LorentzVector& GetFinal(G4int indx) const = 0;
};


/// Factory class
/** Create different types of TntNeutronDecay instances,
 *  based on inputs
 */
class TntNeutronDecayFactory {
public:
	/// Set the type of decay, determines what child class is created
	void SetDecayType(G4String type) { fDecayType = type; }
	/// Returns the decay type
	const G4String& GetDecayType() const { return fDecayType; }
	/// Set an optional parameter to be passed to the created TntNeutronDecay instance
	void SetDecayOption(G4String option, G4double value) { mOptions[option] = value; }
	/// Receive some optional decay parameter
	G4double GetDecayOption(G4String option) const;
	/// Create new instance of TntNeutronDecay
	/** User is responsible for deleting the returned value */
	TntNeutronDecay* Create();
private:
	G4String fDecayType;
	std::map<G4String, G4double> mOptions;
};


/// 'Intermediate' TntNeutronDecay class, implementing many of the details
/// common to all decay types.
/** Still abstract, need to implement Generate() */
class TntNeutronDecayIntermediate : public TntNeutronDecay {
public:
	TntNeutronDecayIntermediate(G4int number_of_neutrons_emitted);
	virtual ~TntNeutronDecayIntermediate();
	virtual void SetInputParticle(const TntParticle* p);
	virtual void SetRngEx(const TntRng* rng) { mRngEx = rng; }
	virtual G4int GetNumberOfNeutrons() const { return mNumberOfNeutronsEmitted; }
	virtual G4bool Generate() = 0;
	virtual void SetDecayParam(const G4String& par, G4double val);
	virtual G4double GetDecayParam(const G4String& par);
	virtual void SetVerboseLevel(G4int level) { fVerb = level; }
	virtual G4int GetVerboseLevel() const { return fVerb; }
protected:
	void SetFinal(G4int indx, const G4LorentzVector& v);
	virtual const G4LorentzVector& GetFinal(G4int indx) const;
private:
	G4int mNumberOfNeutronsEmitted;
	std::map<G4String, G4double> mParams;
	std::vector<G4LorentzVector> mFinal;
	G4int fVerb;
protected:
	G4double mFinalFragMass; // Rest mass of final decay fragment
	const TntParticle* mInitial;
	const TntRng* mRngEx;
};

/// Helper class to calculate neutron evaporation
class TntNeutronEvaporation {
public:
	/// Ctor
	/** \param m0 Mass of initial state (G.S. + Excitation)
	 *  \param mf Mass of final "fragment" state (G.S. + Excitation)
	 *  \param mn "Neutron" mass (can be different, if 2nd particle isn't a neutron)
	 */
	TntNeutronEvaporation(G4double m0, G4double mf, G4double mn):
		mM0(m0), mMf(mf), mMn(mn) { }
	/// Dtor
	~TntNeutronEvaporation() {}
	/// Calculates the neutron evaporation in the CENTER OF MASS frame
	/** \param [out] Frag : Final "fragment" 4-vector in COM
	 *  \param [out] Neut : Final "neutron" 4-vector in COM
	 */
	void operator() (G4LorentzVector* Frag, G4LorentzVector* Neut);
private:
	G4double mM0, mMf, mMn;
};

/// Concrete class for single neutron decay, Breit Wigner
/** Parameters to set are "energy" and "width"
 *  Setting "width" to zero returns a uniform (spike) decay energy
 */
class TntOneNeutronDecay : public TntNeutronDecayIntermediate {
public:
	TntOneNeutronDecay();
	virtual ~TntOneNeutronDecay();
	virtual G4bool Generate();
};

/// Concrete class for two neutron  phase space decay. Includes optional
/// final state interaction (FSI).
/**  If used, FSI is taken from the code by
 *   F. Marquis, sent originally by him to J.K. Smith, then to GAC. 
 *   Reference for the FSI calculation is PLB 476, 219 (2000), 
 *   https://doi.org/10.1016/S0370-2693(00)00141-6
 */
class TntTwoNeutronDecayPhaseSpace : public TntNeutronDecayIntermediate {
public:
	/// Ctor
	/** \param [in] fsi If true, include final state interaction in calculation;
	 *   if false, do not include FSI.
	 */
	TntTwoNeutronDecayPhaseSpace(G4bool fsi = FALSE);
	virtual ~TntTwoNeutronDecayPhaseSpace();
	virtual G4bool Generate();
private:
	G4bool fFSI;
};

/// Concrete class for two neutron 'dineutron' decay
/** Dineutron decay is calculated using the formalism developed
 *  by A. Volya, PRC 76, 064314, 2006 && EPJ Web Conf., 38, 03003, 2012
 */
class TntTwoNeutronDecayDiNeutron : public TntNeutronDecayIntermediate {
public:
	TntTwoNeutronDecayDiNeutron();
	virtual ~TntTwoNeutronDecayDiNeutron();
	virtual G4bool Generate();
};

/// Concrete class for two neutron sequential decay
/** Calculated using the formalism developed
 *  by A. Volya, PRC 76, 064314, 2006 && EPJ Web Conf., 38, 03003, 2012
 */
class TntTwoNeutronDecaySequential : public TntNeutronDecayIntermediate {
public:
	TntTwoNeutronDecaySequential();
	virtual ~TntTwoNeutronDecaySequential();
	virtual void SetInputParticle(const TntParticle* p);
	virtual G4bool Generate();
public:
	G4double mIntermediateFragMass;
};


#endif
