/*
  This file is part of the FRED system.

  Copyright (c) 2010-2015, University of Pittsburgh, John Grefenstette,
  Shawn Brown, Roni Rosenfield, Alona Fyshe, David Galloway, Nathan
  Stone, Jay DePasse, Anuroop Sriram, and Donald Burke.

  Licensed under the BSD 3-Clause license.  See the file "LICENSE" for
  more information.
*/

//
//
// File: VaccineDose.h
//

#ifndef _FRED_VACCINEDOSE_H
#define _FRED_VACCINEDOSE_H

#include "Global.h"
#include "Age_Map.h"

class Vaccine_Dose {
  // Vaccine_Dose is a class to hold the efficacy and dosing information 
  // for a vaccine.  A vaccine may have as many doses as it needs.
public:
  // Creation Operators
  Vaccine_Dose(Age_Map* _efficacy, Age_Map* _efficacy_symp, Age_Map* _efficacy_hosp, Age_Map* _efficacy_delay, Age_Map* _efficacy_duration, Age_Map* _efficacy_hosp_duration, int _days_between_doses, int _mix_match);
  ~Vaccine_Dose();
  
  //Parameter Access
  Age_Map* get_efficacy_map()       const { return efficacy;}
  Age_Map* get_efficacy_symp_map()       const { return efficacy_symp;}
  Age_Map* get_efficacy_hosp_map()       const { return efficacy_hosp;}
  Age_Map* get_efficacy_delay_map() const { return efficacy_delay;}
  
  double  get_efficacy(double real_age) const { return efficacy->find_value(real_age);  }
  double  get_efficacy_symp(double real_age) const { return efficacy_symp->find_value(real_age);  }
  double  get_efficacy_hosp(double real_age) const { return efficacy_hosp->find_value(real_age);  }
  double  get_efficacy_delay(double real_age)   const { return efficacy_delay->find_value(real_age); }
  double  get_duration_of_immunity(double real_age);
  double  get_duration_of_hosp_immunity(double real_age);
  int     get_days_between_doses()  const { return days_between_doses; }
  int     get_next_dose_mix_match() const { return mix_and_match_next_dose; }
  bool    is_within_age(double real_age) const;

  //Utility Functions... no need for update or reset.
  void print() const;
  
private:
  int days_between_doses;       // Number of days until the next dose is administered
  int mix_and_match_next_dose;
  Age_Map* efficacy;            // Age specific efficacy of vaccine, does the dose provide immunity
  Age_Map* efficacy_symp;            // Age specific efficacy of vaccine against symptoms, does the dose provide immunity
  Age_Map* efficacy_hosp;            // Age specific efficacy of vaccine against symptoms, does the dose provide immunity
  Age_Map* efficacy_delay;      // Age specific delay to efficacy, how long does it take to develop immunity
  Age_Map* efficacy_duration;  // Age specific duration of immunity
  Age_Map* efficacy_hosp_duration;  // Age specific duration of immunity
  
protected:
  Vaccine_Dose() { }
};

#endif




