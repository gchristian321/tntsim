/// \file RunAction.hh
/// \brief Definition of the TexanRunAction class
///
#ifndef TexanRunAction_h
#define TexanRunAction_h 1
#include "G4UserRunAction.hh"

class G4Run;


namespace texansim {

/// Run action class
/*! Only exists to return instance of texansim::Run from GenerateRun()
 */
class RunAction : public G4UserRunAction
{
public:
	/// Ctor, empty
	RunAction();
	/// Dtor, empty
	virtual ~RunAction();
	/// Return new texansim::Run
	virtual G4Run* GenerateRun();
	/// Empty
	virtual void BeginOfRunAction(const G4Run*);
	/// Empty
	virtual void EndOfRunAction(const G4Run*);
};

}

#endif

