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
// File: VaccineStatus.cc
//
#include <stdio.h>
#include <assert.h>
#include <iostream>
#include "Random.h"
#include "Vaccine_Health.h"
#include "Vaccine.h"
#include "Disease.h"
#include "Disease_List.h"
#include "Vaccine_Dose.h"
#include "Vaccine_Manager.h"
#include "Health.h"
#include "Person.h"
#include "Global.h"

Vaccine_Health::Vaccine_Health(int _vaccination_day, Vaccine* _vaccine, double _age, 
			       Person * _person, Vaccine_Manager* _vaccine_manager){
  
  vaccine = _vaccine;
  vaccination_day = _vaccination_day;
  person = _person;
  
  double efficacy = vaccine->get_dose(0)->get_efficacy(_age);
  double efficacy_symp = vaccine->get_dose(0)->get_efficacy_symp(_age);
  double efficacy_hosp = vaccine->get_dose(0)->get_efficacy_hosp(_age);  
  double efficacy_delay = vaccine->get_dose(0)->get_efficacy_delay(_age);
  double efficacy_duration = vaccine->get_dose(0)->get_duration_of_immunity(_age);
  double efficacy_hosp_duration = vaccine->get_dose(0)->get_duration_of_hosp_immunity(_age);

  /*
    Boosters 
  double efficacy_bst_symp = vaccine->get_efficacy_bst_symp(_age);
  double efficacy_bst_hosp = vaccine->get_efficacy_bst_hosp(_age);  
  double efficacy_bst_delay = vaccine->get_efficacy_bst_delay(_age);
  double efficacy_bst_duration = vaccine->get_dose(0)->get_duration_of_immunity(_age);
  */
  // vaccine_booster_day[%1$d] = %7$d;  
  // vaccine_booster_efficacy_age_groups[%1$d] = 1 100;
  // vaccine_booster_efficacy_values[%1$d] = 1 %8$.4f;
  
  // vaccine_booster_efficacy_symptoms_age_groups[%1$d] = 1 100;
  // vaccine_booster_efficacy_symptoms_values[%1$d] = 1 %9$.4f;
  
  // vaccine_booster_efficacy_hospitalization_age_groups[%1$d] = 1 100;  
  // vaccine_booster_efficacy_hospitalization_values[%1$d] = 1 %10$.4f;
  
  vaccine_manager = _vaccine_manager;
  vaccination_immunity_loss_day = -1;
  vaccination_hosp_immunity_loss_day = -1;
  vaccination_effective_day = -1;
  vaccination_effective_symp_day = -1;
  vaccination_effective_hosp_day = -1;

  
  effective = false;
  effective_symptoms = false;
  effective_hospitalization = false;

  // decide on efficacy
  /*
if is effective against infection, add immunity loss day,
if is not effective against infection, try against symptoms
if it's not effective against symptoms, try against hospitalization
a person could have protection against infection and hospitalization, as long as it's not effective against symptoms
   */
  /*
We are gonna get rid of if statements so that everyone can be protected against infection, against symptoms, and against hospitalization
   */
  if(Random::draw_random() < efficacy) {
    vaccination_effective_day = vaccination_day + efficacy_delay;
    vaccination_immunity_loss_day = vaccination_effective_day + 1 + efficacy_duration;
  }

  if(Random::draw_random() < efficacy_symp) {
    vaccination_effective_symp_day = vaccination_day + efficacy_delay;
    vaccination_immunity_loss_day = vaccination_effective_symp_day + 1 + efficacy_duration;
  }
  if(Random::draw_random() < efficacy_hosp) {
    vaccination_effective_hosp_day = vaccination_day + efficacy_delay;
    vaccination_hosp_immunity_loss_day = vaccination_effective_hosp_day + 1 + efficacy_hosp_duration;
  }
  
  current_dose = 0;
  days_to_next_dose = -1;
  next_dose_mix_match = -1;
  if(Global::Debug > 1) {
    cout << "Agent: " << person->get_id() << " took dose " << current_dose << " on day "<< vaccination_day << "\n";
  }
  if(vaccine->get_number_doses() > 1){
    days_to_next_dose = vaccination_day + vaccine->get_dose(0)->get_days_between_doses();
    next_dose_mix_match = vaccine->get_dose(0)->get_next_dose_mix_match();
  }
  /*
  printf("Vaccine: %d Efficacy %.2f -> %d, Efficacy Symptoms %.2f -> %d, Efficacy Hosp %.2f -> %d\n",
	 vaccine->get_ID(), efficacy, vaccination_effective_day, efficacy_symp, vaccination_effective_symp_day, efficacy_hosp, vaccination_effective_hosp_day);
  */
}

void Vaccine_Health::print() const {
  // TODO
  cout << "\nVaccine_Status";
}

void Vaccine_Health::printTrace() const {
  fprintf(Global::VaccineTracefp," vaccday %5d age %5.1f iseff %2d iseffsymp %2d ifeffhosp %2d effday %5d currentdose %3d",vaccination_day,
	  person->get_real_age(),is_effective(), is_effective_symptoms(), is_effective_hospitalization(), this->get_vaccination_any_effective_day(), current_dose);
  fflush(Global::VaccineTracefp);
}

void Vaccine_Health::update(int day, double age){
  // First check for immunity
  // CHECK ON VACCINE DIFFERENTIAL EFFICACY AGAINST DISEASE IDS
  if (is_effective()) {
    if (day == vaccination_effective_day) {
      for(int dis_id = 0; dis_id < Global::Diseases.get_number_of_diseases(); ++dis_id){
	if(Global::Enable_Disease_Cross_Protection == false && dis_id > 0){
	  break;
	}
	Disease* disease = Global::Diseases.get_disease(dis_id);
	if (person->is_infected(disease->get_id())==false) {
	  if(Global::Enable_Disease_Cross_Protection == true && dis_id > 0){
	    if(Random::draw_random() <  vaccine->get_disease_specific_efficacy(dis_id)) {
	      person->become_immune(disease);
	    }
	  }else{
	    person->become_immune(disease);
	  }
	  effective = true;
	  if(Global::Verbose > 0) {
	    cout << "Agent " << person->get_id() 
		 << " has become immune from dose "<< current_dose 
		 << " on day " << day << "\n";
	  }
	}
	else {
	  if(Global::Verbose > 0) {
	    cout << "Agent " << person->get_id() 
		 << " was already infected so did not become immune from dose "<< current_dose 
		 << " on day " << day << "\n";
	  }
	}
      }
    }
    if (day == vaccination_immunity_loss_day) {      
      if(Global::Verbose > 0) {
	cout << "Agent " << person->get_id() 
	     << " became immune on day "<< vaccination_effective_day
	     << " and lost immunity on day " << day << " duration " <<  "\n";
      }
      /*
	If a person is currently infected, don't clear vaccine immunity, natural imunity would take care of it. 
       */
      bool current_infected = false;
      for(int dis_id = 0; dis_id < Global::Diseases.get_number_of_diseases(); ++dis_id){
	if(person->is_infected(dis_id) == true){
	  current_infected = true;
	}
      }      
      if(current_infected == false){
	if(Global::Enable_Disease_Cross_Protection == true && Global::Diseases.get_number_of_diseases() > 1){
	  for(int dis_id = 0; dis_id < Global::Diseases.get_number_of_diseases(); ++dis_id){
	    person->become_susceptible_by_vaccine_waning(dis_id);
	  }
	}else{
	  int disease_id = 0;
	  person->become_susceptible_by_vaccine_waning(disease_id);
	}
      }
      effective = false;
    }
  }
  
  // Next check on symptoms
  if (is_effective_symptoms()) {
    if (day == vaccination_effective_symp_day) {
      for(int dis_id = 0; dis_id < Global::Diseases.get_number_of_diseases(); ++dis_id){
	if(Global::Enable_Disease_Cross_Protection == false && dis_id > 0){
	  break;
	}
      // Include cross protection here instead of in Health.cc
	Disease* disease = Global::Diseases.get_disease(dis_id);
	if (person->is_infected(disease->get_id())==false) {
	  // Make person become immune to symptoms
	  if(Random::draw_random() <  vaccine->get_disease_specific_efficacy_symp(dis_id)) {
	    person->become_immune_to_symptoms(disease);
	  }else{
	    person->become_immune_to_symptoms(disease);
	  }
	  
	  effective_symptoms = true;
	
	  if(Global::Verbose > 0) {
	    cout << "Agent " << person->get_id() 
		 << " has become immune to symptoms from dose "<< current_dose 
		 << " on day " << day << "\n";
	  }
	}
	else {
	  if(Global::Verbose > 0) {
	    cout << "Agent " << person->get_id() 
		 << " was already infected so did not become immune to symptoms from dose "<< current_dose 
		 << " on day " << day << "\n";
	  }
	}
      }
    }
    if (day == vaccination_immunity_loss_day) {
      if(Global::Verbose > 0) {
	cout << "Agent " << person->get_id() 
	     << " became immune on day "<< vaccination_effective_symp_day
	     << " and lost immunity on day " << day << "\n";
      }
      /*
	This looks weird considering there are more than one disease_id
      */
      if(Global::Enable_Disease_Cross_Protection == true && Global::Diseases.get_number_of_diseases() > 1){
	for(int dis_id = 0; dis_id < Global::Diseases.get_number_of_diseases(); ++dis_id){
	  // TODO: Become susceptible to symptoms due to vaccine waning
	  person->become_susceptible_to_symptoms_by_vaccine_waning(dis_id);
	}
      }else{
	int disease_id = 0;
	// TODO: Become susceptible to symptoms due to vaccine waning
	person->become_susceptible_to_symptoms_by_vaccine_waning(disease_id);
      }
      effective_symptoms = false;
    }
  }

  // Next check on severe disease
  if (is_effective_hospitalization()) {
    if (day == vaccination_effective_hosp_day) {
      for(int dis_id = 0; dis_id < Global::Diseases.get_number_of_diseases(); ++dis_id){
	if(Global::Enable_Disease_Cross_Protection == false && dis_id > 0){
	  break;
	}
      // Include cross protection here instead of in Health.cc
	Disease* disease = Global::Diseases.get_disease(dis_id);
	if (person->is_infected(disease->get_id())==false) {
	  // Make person become immune to symptoms
	  if(Random::draw_random() <  vaccine->get_disease_specific_efficacy_hosp(dis_id)) {
	    person->become_immune_to_hospitalization(disease);
	  }else{
	    person->become_immune_to_hospitalization(disease);
	  }
	
	  effective_hospitalization = true;
	
	  if(Global::Verbose > 0) {
	    cout << "Agent " << person->get_id() 
		 << " has become immune to hospitalization from dose "<< current_dose 
		 << " on day " << day << "\n";
	  }
	}
	else {
	  if(Global::Verbose > 0) {
	    cout << "Agent " << person->get_id() 
		 << " was already infected so did not become immune to hospitalization from dose "<< current_dose 
		 << " on day " << day << "\n";
	  }
	}
      }
    }
    if (day == vaccination_hosp_immunity_loss_day) {
      if(Global::Verbose > 0) {
	cout << "Agent " << person->get_id() 
	     << " became immune on day "<< vaccination_effective_hosp_day
	     << " and lost immunity on day " << day << "\n";
      }
      /*
	This looks weird considering there are more than one disease_id
      */
      if(Global::Enable_Disease_Cross_Protection == true && Global::Diseases.get_number_of_diseases() > 1){
	for(int dis_id = 0; dis_id < Global::Diseases.get_number_of_diseases(); ++dis_id){
	  person->become_susceptible_to_hospitalization_by_vaccine_waning(dis_id);
	}
      }else{
	int disease_id = 0;
	person->become_susceptible_to_hospitalization_by_vaccine_waning(disease_id);
      }
      effective_hospitalization = false;
    }
  }
  
  
  // Next check on dose
  // Even immunized people get another dose
  // If they are to get another dose, then put them on the queue based on dose priority
 
  if (current_dose < vaccine->get_number_doses()-1) {   // Are we done with the dosage?
    // Get the dosage policy from the manager
    if(day >= days_to_next_dose){
      current_dose++;
      days_to_next_dose = day + vaccine->get_dose(current_dose)->get_days_between_doses();
      next_dose_mix_match =  vaccine->get_dose(current_dose)->get_next_dose_mix_match();
      int vaccine_dose_priority = vaccine_manager->get_vaccine_dose_priority();
      if(Global::Debug < 1){
	cout << "Agent " << person->get_id()
	     << " being put in to the queue with priority " << vaccine_dose_priority
	     << " for dose " << current_dose 
	     << " on day " << day << "\n";
      }
      switch(vaccine_dose_priority){
      case VACC_DOSE_NO_PRIORITY:
	vaccine_manager->add_to_regular_queue_random(person);
	break;
      case VACC_DOSE_FIRST_PRIORITY:
	vaccine_manager->add_to_priority_queue_begin(person);
	break;
      case VACC_DOSE_RAND_PRIORITY:
	vaccine_manager->add_to_priority_queue_random(person);
	break;
      case VACC_DOSE_LAST_PRIORITY:
	vaccine_manager->add_to_priority_queue_end(person);
      case VACC_DOSE_SEPARATE_PRIORITY:
	vaccine_manager->add_to_next_dose_queue_end(person);
	break;
  
      }
    }
  }
}


void Vaccine_Health::update_for_next_dose(int day, double age){
  /*
TODO: It seems like new doses only updae if not effective. 
In reality, Not just efficacy can be boosted by a new dose but maybe the duration of immunity.
 */
  vaccination_day = day;
  if(!is_effective()){
    double efficacy = vaccine->get_dose(current_dose)->get_efficacy(age);
    double efficacy_symp = vaccine->get_dose(current_dose)->get_efficacy_symp(age);
    double efficacy_hosp = vaccine->get_dose(current_dose)->get_efficacy_hosp(age);
    double efficacy_delay = vaccine->get_dose(current_dose)->get_efficacy_delay(age);
    double efficacy_duration = vaccine->get_dose(current_dose)->get_duration_of_immunity(age);
    double efficacy_hosp_duration = vaccine->get_dose(current_dose)->get_duration_of_hosp_immunity(age);
    if(vaccination_effective_day == -1){
      if (Random::draw_random() < efficacy){
	vaccination_effective_day = day + efficacy_delay;
	vaccination_immunity_loss_day = vaccination_effective_symp_day + 1 + efficacy_duration;      
      }
    }
    if(vaccination_effective_symp_day == -1){
      if(Random::draw_random() < efficacy_symp) {
	vaccination_effective_symp_day = vaccination_day + efficacy_delay;
	vaccination_immunity_loss_day = vaccination_effective_symp_day + 1 + efficacy_duration;      
      }
    }
    if(vaccination_effective_hosp_day == -1){
      if(Random::draw_random() < efficacy_hosp) {
	vaccination_effective_hosp_day = vaccination_day + efficacy_delay;
	vaccination_hosp_immunity_loss_day = vaccination_effective_hosp_day + 1 + efficacy_hosp_duration;      
      }
    }
    /*
    printf("Vaccine Dose: %d of ID %d Efficacy %.2f -> %d, Efficacy Symptoms %.2f -> %d, Efficacy Hosp %.2f -> %d\n",
	   current_dose,
	   vaccine->get_ID(), efficacy, vaccination_effective_day, efficacy_symp, vaccination_effective_symp_day, efficacy_hosp, vaccination_effective_hosp_day);
    */
  }
}
