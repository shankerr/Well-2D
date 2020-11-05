/*
###############################################################################
# If you use PhysiCell in your project, please cite PhysiCell and the version #
# number, such as below:                                                      #
#                                                                             #
# We implemented and solved the model using PhysiCell (Version x.y.z) [1].    #
#                                                                             #
# [1] A Ghaffarizadeh, R Heiland, SH Friedman, SM Mumenthaler, and P Macklin, #
#     PhysiCell: an Open Source Physics-Based Cell Simulator for Multicellu-  #
#     lar Systems, PLoS Comput. Biol. 14(2): e1005991, 2018                   #
#     DOI: 10.1371/journal.pcbi.1005991                                       #
#                                                                             #
# See VERSION.txt or call get_PhysiCell_version() to get the current version  #
#     x.y.z. Call display_citations() to get detailed information on all cite-#
#     able software used in your PhysiCell application.                       #
#                                                                             #
# Because PhysiCell extensively uses BioFVM, we suggest you also cite BioFVM  #
#     as below:                                                               #
#                                                                             #
# We implemented and solved the model using PhysiCell (Version x.y.z) [1],    #
# with BioFVM [2] to solve the transport equations.                           #
#                                                                             #
# [1] A Ghaffarizadeh, R Heiland, SH Friedman, SM Mumenthaler, and P Macklin, #
#     PhysiCell: an Open Source Physics-Based Cell Simulator for Multicellu-  #
#     lar Systems, PLoS Comput. Biol. 14(2): e1005991, 2018                   #
#     DOI: 10.1371/journal.pcbi.1005991                                       #
#                                                                             #
# [2] A Ghaffarizadeh, SH Friedman, and P Macklin, BioFVM: an efficient para- #
#     llelized diffusive transport solver for 3-D biological simulations,     #
#     Bioinformatics 32(8): 1256-8, 2016. DOI: 10.1093/bioinformatics/btv730  #
#                                                                             #
###############################################################################
#                                                                             #
# BSD 3-Clause License (see https://opensource.org/licenses/BSD-3-Clause)     #
#                                                                             #
# Copyright (c) 2015-2018, Paul Macklin and the PhysiCell Project             #
# All rights reserved.                                                        #
#                                                                             #
# Redistribution and use in source and binary forms, with or without          #
# modification, are permitted provided that the following conditions are met: #
#                                                                             #
# 1. Redistributions of source code must retain the above copyright notice,   #
# this list of conditions and the following disclaimer.                       #
#                                                                             #
# 2. Redistributions in binary form must reproduce the above copyright        #
# notice, this list of conditions and the following disclaimer in the         #
# documentation and/or other materials provided with the distribution.        #
#                                                                             #
# 3. Neither the name of the copyright holder nor the names of its            #
# contributors may be used to endorse or promote products derived from this   #
# software without specific prior written permission.                         #
#                                                                             #
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" #
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE   #
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE  #
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE   #
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR         #
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF        #
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS    #
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN     #
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)     #
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE  #
# POSSIBILITY OF SUCH DAMAGE.                                                 #
#                                                                             #
###############################################################################
*/

#include "./custom.h"

// declare cell definitions here 

Cell_Definition fibroblast; 
Cell_Definition KRAS_negative; 
Cell_Definition KRAS_positive; 

void create_cell_types( void )
{
	// use the same random seed so that future experiments have the 
	// same initial histogram of oncoprotein, even if threading means 
	// that future division and other events are still not identical 
	// for all runs 
	
	SeedRandom( parameters.ints("random_seed") ); // or specify a seed here 
	
	// housekeeping 
	
	initialize_default_cell_definition();
	cell_defaults.phenotype.secretion.sync_to_microenvironment( &microenvironment ); 
	
	// Name the default cell type 
	
	cell_defaults.type = 0; 
	cell_defaults.name = "default cell"; 
	

	
	// set default_cell_functions; 
	
	cell_defaults.functions.update_phenotype = NULL; 
	
	// needed for a 2-D simulation: 
	
	/* grab code from heterogeneity */ 
	
	cell_defaults.functions.set_orientation = up_orientation; 
	cell_defaults.phenotype.geometry.polarity = 1.0;
	cell_defaults.phenotype.motility.restrict_to_2D = true; 
	
	// make sure the defaults are self-consistent. 
	
	cell_defaults.phenotype.secretion.sync_to_microenvironment( &microenvironment );
	cell_defaults.phenotype.molecular.sync_to_microenvironment( &microenvironment );	
	cell_defaults.phenotype.sync_to_functions( cell_defaults.functions ); 

	// set the rate terms in the default phenotype 

	// first find index for a few key variables. 
	int apoptosis_model_index = cell_defaults.phenotype.death.find_death_model_index( "Apoptosis" );
	int necrosis_model_index = cell_defaults.phenotype.death.find_death_model_index( "Necrosis" );

	// initially no necrosis 
	cell_defaults.phenotype.death.rates[necrosis_model_index] = 0.0; 
    cell_defaults.phenotype.death.rates[apoptosis_model_index] = 1.0; 


	// set default cell cycle model 

	cell_defaults.functions.cycle_model = live; 
	int start_index = live.find_phase_index( PhysiCell_constants::live );
	int end_index = live.find_phase_index( PhysiCell_constants::live );
    cell_defaults.phenotype.cycle.data.transition_rate(start_index,end_index) = 0.0;

    // metabolite indices
    int oxygen_substrate_index = microenvironment.find_density_index( "oxygen" );
    int glucose_substrate_index = microenvironment.find_density_index( "glucose" );
    int glutamine_substrate_index = microenvironment.find_density_index( "glutamine" );
    int lactate_substrate_index = microenvironment.find_density_index( "lactate" );
    

	// set oxygen uptake / secretion parameters for the default cell type 
	cell_defaults.phenotype.secretion.uptake_rates[oxygen_substrate_index] = 0.0; 
	cell_defaults.phenotype.secretion.secretion_rates[oxygen_substrate_index] = 0; 
	cell_defaults.phenotype.secretion.saturation_densities[oxygen_substrate_index] = 0.0; 
	
    cell_defaults.phenotype.secretion.uptake_rates[glucose_substrate_index] = 0.0; 
	cell_defaults.phenotype.secretion.secretion_rates[glucose_substrate_index] = 0; 
	cell_defaults.phenotype.secretion.saturation_densities[glucose_substrate_index] = 0.0; 
    
	cell_defaults.phenotype.secretion.uptake_rates[glutamine_substrate_index] = 0.0; 
	cell_defaults.phenotype.secretion.secretion_rates[glutamine_substrate_index] = 0; 
	cell_defaults.phenotype.secretion.saturation_densities[glutamine_substrate_index] = 0.0; 

	cell_defaults.phenotype.secretion.uptake_rates[lactate_substrate_index] = 0.0; 
	cell_defaults.phenotype.secretion.secretion_rates[lactate_substrate_index] = 0; 
	cell_defaults.phenotype.secretion.saturation_densities[lactate_substrate_index] = 0.0;     
    
    
	// no motility
	cell_defaults.phenotype.motility.is_motile = false;

	// add custom data here, if any 
    cell_defaults.custom_data.add_variable( "organoid_number" , "dimensionless", 0.0 );
    cell_defaults.custom_data.add_variable( "oxygen_i_conc" , "mmHg", 0.0 );
    cell_defaults.custom_data.add_variable( "glucose_i_conc" , "mMolar", 0.0 );
    cell_defaults.custom_data.add_variable( "glutamine_i_conc", "mMolar", 0.0);
    cell_defaults.custom_data.add_variable( "lactate_i_conc" , "mMolar", 0.0 ); 
    cell_defaults.custom_data.add_variable( "energy", "a.u", 0.0);
		
    
    // Definition of Fibroblast
    fibroblast = cell_defaults;
    fibroblast.type = 1;
    fibroblast.name = "fibroblast";
    
    // making sure that cells copy the paramters from cell_defaults
    fibroblast.parameters.pReference_live_phenotype = &( fibroblast.phenotype ); 
    
    // lactate uptake rate
    fibroblast.phenotype.secretion.uptake_rates[lactate_substrate_index] = 0.1; 


    // Create KRAS_negative and KRAS_positive types
    
    // Definition of KRAS negative
    KRAS_negative = cell_defaults;
    KRAS_negative.type = 2;
    
    // KRAS negative uptake rates for glucose, glutamine, and oxygen
    KRAS_negative.phenotype.secretion.uptake_rates[oxygen_substrate_index] = 0.1;
    KRAS_negative.phenotype.secretion.uptake_rates[glucose_substrate_index] = 0.01;
    KRAS_negative.phenotype.secretion.uptake_rates[glutamine_substrate_index] = 0.0001;
    
    // KRAS negative lactate secretion
    KRAS_negative.phenotype.secretion.secretion_rates[lactate_substrate_index] = 0.001; 
    KRAS_negative.phenotype.secretion.saturation_densities[lactate_substrate_index] = 10.0;     
    
    
    
    // Definition of KRAS positive
    KRAS_positive = cell_defaults;
    KRAS_positive.type = 3;

    // KRAS positive uptake rates for glucose, glutamine, and oxygen
    KRAS_positive.phenotype.secretion.uptake_rates[oxygen_substrate_index] = 0.5;
    KRAS_positive.phenotype.secretion.uptake_rates[glucose_substrate_index] = 0.05;
    KRAS_positive.phenotype.secretion.uptake_rates[glutamine_substrate_index] = 0.0005;
	
    // KRAS positve lactate secretion
    KRAS_positive.phenotype.secretion.secretion_rates[lactate_substrate_index] = 0.005; 
    KRAS_positive.phenotype.secretion.saturation_densities[lactate_substrate_index] = 20.0;  


    //

		
	build_cell_definitions_maps(); 
	display_cell_definitions( std::cout ); 
	
	return; 
}

void setup_microenvironment( void )
{
	// set domain parameters 
	
/* now this is in XML 
	default_microenvironment_options.X_range = {-1000, 1000}; 
	default_microenvironment_options.Y_range = {-1000, 1000}; 
	default_microenvironment_options.simulate_2D = true; 
*/
	
	// make sure to override and go back to 2D 
	if( default_microenvironment_options.simulate_2D == false )
	{
		std::cout << "Warning: overriding XML config option and setting to 2D!" << std::endl; 
		default_microenvironment_options.simulate_2D = true; 
	}
	
/* now this is in XML 	
	// no gradients need for this example 

	default_microenvironment_options.calculate_gradients = false; 
	
	// set Dirichlet conditions 

	default_microenvironment_options.outer_Dirichlet_conditions = true;
	
	// if there are more substrates, resize accordingly 
	std::vector<double> bc_vector( 1 , 38.0 ); // 5% o2
	default_microenvironment_options.Dirichlet_condition_vector = bc_vector;
	
	// set initial conditions 
	default_microenvironment_options.initial_condition_vector = { 38.0 }; 
*/
	
	// put any custom code to set non-homogeneous initial conditions or 
	// extra Dirichlet nodes here. 
	
	// initialize BioFVM 
	
	initialize_microenvironment(); 	
	
	return; 
}

void setup_tissue( void )
{
	// create some cells near the origin
	
	Cell* pC;

	// now create a fibroblast cell 
	
	pC = create_cell( fibroblast ); 
	pC->assign_position( 15.0, -18.0, 0.0 );
    
    
	
	return; 
}

std::vector<std::string> my_coloring_function( Cell* pCell )
{
	// start with flow cytometry coloring 
	
	std::vector<std::string> output = false_cell_coloring_cytometry(pCell); 
		
	if( pCell->phenotype.death.dead == false && pCell->type == 1 )
	{
		 output[0] = "blue"; 
		 output[2] = "black"; 
	}
	
	return output; 
}
