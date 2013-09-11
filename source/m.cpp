
#include <boost/cstdint.hpp>

#include "../include/debugger.hpp"

#include "../include/std_headers.hpp"
#include "../include/neural_net_headers.hpp"
#include "../include/data_parser.hpp"

#include "configuration.hpp"

// PMAP is used just to hold program for debugging memory usage
#ifdef PMAP
#include <cstdio>
#endif //PMAP

using namespace boost;

/**
 * \mainpage Kohonen Neural Network Library Demo
 *
 * \par Program is designed for making pattern recognition using Kohonen Self-oragnizing Map.
 *
 * \section intro_sec Introduction
 *
 * \section options Options
 *
 * \par Program usage:
 *
 * knnl_demo [OPTIONS] [FILE]
 *
 * If FILE is not given then standard input (stdin) is taken as input stream.
 *
 * \par Generic options:
 *
 * - -h help message
 *
 * - -V version string
 *
 * - -r number of rows in neural matrix
 *
 * - -c number of columns in neural martix
 *
 * - -e number of epochs that training process will pass
 *
 * \par Hidden options:
 *
 * - -i [ --input-file ] arg input file name
 *
* \author Seweryn Habdank-Wojewodzki
* \author Janusz Rybarski
*/

/**
 * \file main.cpp
 * \brief Main file of the program contains int32_t main ( int32_t argc, char * argv[] ) function :-)
 */

/**
 * \var DEBUGGER_STREAM
 * \brief Set dynamical pointer to the stream.
 * \code
 * DEBUGGER_STREAM =
 *    ::std::auto_ptr < ::std::ofstream > ( new ::std::ofstream ( "_debugger.out" ) );
 * \endcode
 */
#ifdef FTDEBUG
::std::auto_ptr < ::std::ofstream > DEBUGGER_STREAM;
#endif //FTDEGUG

/**
 * \defgroup data_storages Data storages
 */

/**
 * Configuration.
 * \ingroup data_storages
 */
::config::Configuration CONF;

/**
 * Main function of the program.
 */



using namespace std;


typedef double data_type;
typedef ::std::vector < data_type > V_d;
typedef ::std::vector < V_d > V_v_d;



void read_input(V_v_d &data, string file_name){
	::data_parser::Data_parser < V_v_d > data_parser;	
	stringstream tmp_stream;
	
	istream *file_ptr = static_cast <istream * > ( 0 );
	file_ptr = new ifstream (file_name.c_str());
	if ( !file_ptr ) {
		cout << "Error in reading file." << endl; 
		exit ( EXIT_FAILURE );
	}

        tmp_stream << file_ptr->rdbuf();
        
        
        delete file_ptr;       
        
        data_parser ( tmp_stream, data );
} 
 
int32_t main ()
{

		int32_t R = 5;
		int32_t C = 5;
		int32_t const no_epochs = 10;


		V_v_d data, data_joints;  
		read_input(data, "hands"); 


            // configure Cauchy hat function
            typedef ::neural_net::Cauchy_function < V_d::value_type, V_d::value_type, int32_t > C_a_f;
            C_a_f c_a_f ( 2.0, 1 );
            typedef ::neural_net::Gauss_function < V_d::value_type, V_d::value_type, int32_t > G_a_f;
            G_a_f g_a_f ( 2.0, 1 );
            typedef ::distance::Euclidean_distance_function < V_d > E_d_t;
            E_d_t e_d;
            typedef ::distance::Weighted_euclidean_distance_function < V_d, V_d > We_d_t;
            typedef ::neural_net::Basic_neuron < C_a_f, E_d_t > Kohonen_neuron;
            Kohonen_neuron my_neuron ( *(data.begin()), c_a_f, e_d );


            /* neural networks parametrized by different neurons */

            // Kohonen_neuron is used for network construction
            typedef ::neural_net::Rectangular_container < Kohonen_neuron > Kohonen_network;
            Kohonen_network kohonen_network_2;

            // prepare randomization policy
            ::neural_net::Internal_randomize IR;

            // generate networks initialized by data
            ::neural_net::generate_kohonen_network
                ( R, C, c_a_f, e_d, data, kohonen_network_2, IR );
            typedef ::neural_net::Wta_proportional_training_functional < V_d, double, int32_t > Wta_train_func;

            // define proper functionals
            Wta_train_func wta_train_func ( 0.2, 0 );

            typedef ::neural_net::Wta_training_algorithm < 
                    Kohonen_network, V_d, V_v_d::iterator, Wta_train_func > Learning_algorithm;


            // Hexagonal topology will be used in further tests
            typedef ::neural_net::Hexagonal_topology < int32_t > Hex_top;
            Hex_top hex_top ( kohonen_network_2.get_no_rows() );
            typedef ::neural_net::Gauss_function < V_d::value_type, V_d::value_type, int32_t > G_f_space;
            G_f_space g_f_space ( 100, 1 );
            typedef ::neural_net::Cauchy_function < V_d::value_type, V_d::value_type, int32_t > C_f_space;
            C_f_space c_f_space ( 100, 1 );
            typedef ::neural_net::Constant_function < V_d::value_type, V_d::value_type > Co_f_space;
            Co_f_space co_f_space ( 1 );
            typedef ::neural_net::Gauss_function < int32_t, V_d::value_type, int32_t > G_f_net;
            G_f_net g_f_net ( 10, 1 );

// typedefs for tests
typedef G_f_space Space_func;
typedef G_f_net Net_func;
typedef Hex_top Net_top;
typedef E_d_t Space_top;

            // constructing typical training generalized weight
            typedef ::neural_net::Classic_training_weight
                <
                V_d,
                int32_t,
                Net_func,
                Space_func,
                Net_top,
                Space_top,
                int32_t
                    > Classic_weight;

            // define if
            Classic_weight classic_weight ( g_f_net, g_f_space, hex_top, e_d );
            typedef ::neural_net::Wtm_classical_training_functional
                <
                V_d,
                double,
                int32_t,
                int32_t,
                Classic_weight
                    > Wtm_c_l_f;

            // definition
            Wtm_c_l_f wtm_c_l_f ( classic_weight, 0.3 );

            ::std::cout << "kohonen_network_2" << ::std::endl;
                        ::neural_net::print_network_weights ( ::std::cout, kohonen_network_2 );
            ::std::cout << ::std::endl;

            // construction of algorithm
            typedef ::neural_net::Wtm_training_algorithm
                <
                Kohonen_network,
                V_d,
                V_v_d::iterator,
                Wtm_c_l_f,
                int32_t
                    > Wtm_c_training_alg;

            // definition
            ::std::cout << "training" << ::std::endl;
            Wtm_c_training_alg wtm_c_train_alg ( wtm_c_l_f );

            // tricky training
            for ( int32_t i = 0; i < no_epochs; ++i )
            {
                wtm_c_train_alg ( data.begin(), data.end(), &kohonen_network_2 );
                wtm_c_train_alg.training_functional.generalized_training_weight.network_function.sigma *= 2.0/3.0;
		random_shuffle ( data.begin(), data.end() );
            }
		::neural_net::print_network ( ::std::cout, kohonen_network_2, *(data.begin()) );
		
            ::std::cout << "kohonen_network_2_post" << ::std::endl;
            ::neural_net::print_network_weights ( ::std::cout, kohonen_network_2 );
            ::std::cout << ::std::endl;


    return 0;
}
