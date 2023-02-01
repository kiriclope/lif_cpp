#ifndef __VOLTUTILS__
#define __VOLTUTILS__


float print_PSP(int i_pop, int j_pop) {
  float tp, vpsp ;
  tp = log(TAU_MEM[i_pop]/TAU_SYN[j_pop + i_pop * n_pop])/(1.0/TAU_MEM[i_pop]-1.0/TAU_SYN[j_pop + i_pop * n_pop]) ; 
  vpsp = exp(-tp/TAU_SYN[j_pop + i_pop * n_pop]) ; 
  vpsp = 20.0 * vpsp / Vth ;  
  return vpsp ;
}
  

void open_lif_files() {
  string str_volt = path + "/mem_volt.dat" ;
  file_volt.open(str_volt.c_str(), ios::out | ios::ate); 
  
  string str_spike_times = path + "/spike_times.dat" ; 
  file_spike_times.open(str_spike_times.c_str(), ios::out | ios::ate);   
}


void close_lif_files() {
  file_volt.close() ;
  file_spike_times.close() ;
}

void save_ff_inputs() {

  file_ff_inputs << t_time - TIME_STEADY ;
  
  for(i=0;i<n_neurons;i++) { 
    file_ff_inputs << " " << filter_ff_inputs[i]*DT/TIME_WINDOW ; 
    filter_ff_inputs[i] = 0 ; 
  } 
  file_ff_inputs << endl ; 
}

void save_volt() {

  file_volt << t_time - TIME_STEADY ;
  
  for(i=0;i<n_neurons;i++) { 
    if(t_spike[i_neuron] == t_time) 
      file_volt << " " << Vpeak ; 
    else 
      file_volt << " " << volt[i] ; 
  } 
  file_volt << endl ; 
  
}


void init_ksi_init(float mean, float var) { 
  
  for(i=0; i<n_pop; i++) 
    for(j=cum_n_per_pop[i]; j<cum_n_per_pop[i+1]; j++) 
      ksi_init[j] = mean + sqrt(var) * white_noise(ksi_gen) ; 
} 


void scale_ksi() { // ONLY USE IF CONN PROB NOT CHANGED IN con_mat.h
  cout << "Scaling low_rank synapses... " << endl ;

  float ksi_dum ;
  int dum ;
  unsigned long counter=0 ;
  
  for(i=0; i<n_pop; i++) // post
    for(j=0; j<n_pop; j++) { // pre
      dum = j+i*n_pop ; 
      if(IS_STRUCT_SYN[dum]) 
	for(k=cum_n_per_pop[i]; k<cum_n_per_pop[i+1]; k++) 	
	  for(l=cum_n_per_pop[j]; l<cum_n_per_pop[j+1]; l++) {
	    
	    ksi_dum = J[dum] / sqrt_Ka[j] ; 
	    ksi_dum += KAPPA * ksi[k]*ksi[l] / K ; 
	    
	    if(RANK==2) 
	      ksi_dum += KAPPA_1 * ksi_1[k]*ksi_1[l] / K ;
	    
	    ksi_scaled[k+l*n_neurons] = GAIN * cut_LR( ksi_dum ) ; 
	    if(IF_SYN_DYN) ksi_scaled[k+l*n_neurons] /= TAU_SYN[dum] ; 
	    if(IF_RESCALE) ksi_scaled[k+l*n_neurons] *= (Vth-Vr[j]) ; 
	    
	    if(ksi_scaled[k+l*n_neurons]==0) counter++ ; 	    
	    if(k<3 && l<3) cout << ksi_scaled[k+l*n_per_pop[0]] << " " ; 
	  }

      cout << endl ; 
      cout << "zeros: " << counter / n_per_pop[i]  << endl ; 
      
    } 
  cout << "Done" << endl ;       
}

void scale_J() {
  cout << "1/sqrt(K) " << 1.0/sqrt_K << endl ; 
  cout << "Scaling J ... " << endl ; 
  for(i=0;i<n_pop;i++) { // postsynaptic 
    for(j=0;j<n_pop;j++) { // presynaptic 
      J_scaled[j+i*n_pop] = GAIN * J[j+i*n_pop] / sqrt_Ka[j] ; 
      if(IF_SYN_DYN) { 
	J_scaled[j+i*n_pop] /= TAU_SYN[j+i*n_pop] ; 
	// might be the right thing given my implementation 
	J_scaled[j+i*n_pop] *= EXP_DT_TAU_SYN[j+i*n_pop] ;
      } 
      if(IF_MATO_K) J_scaled[j+i*n_pop] *= sqrt_Ka[0] / sqrt_Ka[j] ; 
      if(IF_RESCALE) J_scaled[j+i*n_pop] *= (Vth-Vr[j]) ; 
      cout << "pre " << j << " post " << i << " Jij " << J[j+i*n_pop] ; 
      cout << " Jij_scaled " << J_scaled[j+i*n_pop] ; 
      cout << " PSP " << J_scaled[j+i*n_pop] * print_PSP(i,j) << endl ; 
    } 
  } 
  /* if(IF_NMDA) { */ 
  /*   /\* J_scaled[0] *= R_NMDA[0] / (1.0 + R_NMDA[0]) ;  *\/ */
  /*   /\* J_scaled[2] *= R_NMDA[1] / (1.0 + R_NMDA[1]) ;  *\/ */
  /* }   */
}

void scale_J_nmda() { 
  cout << "Scaling J_nmda ... " << endl ; 
  for(i=0;i<n_pop;i++) { // postsynaptic 
    for(j=0;j<n_pop;j++) { // presynaptic 
      J_nmda[j+i*n_pop] = GAIN * J[j+i*n_pop] / sqrt_Ka[j] ; 
      J_nmda[j+i*n_pop] /= TAU_NMDA[j+i*n_pop] ; 
      J_nmda[j+i*n_pop] *= EXP_DT_TAU_NMDA[j+i*n_pop] ;
      if(IF_MATO_K) J_nmda[j+i*n_pop] *= sqrt_Ka[0] / sqrt_Ka[j] ; 
      if(IF_RESCALE) J_nmda[j+i*n_pop] *= (Vth-Vr[j]) ; 
      cout << "pre " << j << " post " << i << " Jij " << J[j+i*n_pop] ; 
      cout << " Jij_nmda " << J_nmda[j+i*n_pop] ; 
      cout << " PSP " << J_nmda[j+i*n_pop] * print_PSP(i,j) << endl ; 
    } 
  } 
  J_nmda[0] *= 0.92 ; // 1.0 / (1.0 + R_NMDA[0]) ; 
  J_nmda[2] *= 7.4 / J[2] ; //1.0 / (1.0 + R_NMDA[1]) ; 
  J_nmda[3] = 0.0 ; // 1.0 / (1.0 + R_NMDA[1]) ; 
  J_nmda[4] = 0.0 ; // 1.0 / (1.0 + R_NMDA[1]) ; 
} 

void scale_ext_inputs() {
  cout << "sqrt(K) " << sqrt_K << endl ; 
  cout << "Scaling ext_inputs ... " << endl ;
  
  for(i=0;i<n_pop;i++) { // postsynaptic 
    ext_inputs_scaled[i] = ext_inputs[i] * sqrt_Ka[0] * m0 ; 
    if(IF_RESCALE) ext_inputs_scaled[i] *= (Vth-Vr[i]) ; 
    cout << "raw " << ext_inputs[i] << " scaled " << ext_inputs_scaled[i] << " " ; 
  } 
  cout << endl ; 

  cout << "var_ff " << endl ;
  if(IF_SQRT_K_NOISE)
    for(i=0;i<n_pop;i++) { 
      var_ff[i] = sigma_FF[i] * sqrt_Ka[0] ; 
      cout << i << " raw " << sigma_FF[i] << " scaled " << var_ff[i] << " " ; 
    }
  else
    for(i=0;i<n_pop;i++) { 
      var_ff[i] = sigma_FF[i] ; 
      cout << i << " raw " << sigma_FF[i] << " scaled " << var_ff[i] << " " ; 
    }    
  cout << endl ; 
  
  for(i=0; i<n_neurons; i++) 
    bg_inputs[i] = (1.0 - IF_FF_LAYER) * ext_inputs_scaled[which_pop[i]] ; 
  
  if(IF_FF_LAYER) {
    if(IF_SPARSE_FF) {

      cout << "var_ff " << endl ; 
      for(i=0;i<n_pop;i++) { 
	var_ff[i] = sigma_FF[i] / sqrt(K_FF) ; 
	cout << i << " raw " << sigma_FF[i] << " scaled " << var_ff[i] << " " ; 
      } 
      cout << endl ; 
      
      for(i=0; i<n_neurons; i++) {
    	/* J_FF[i] = GAIN_FF * ext_inputs[which_pop[i]] / TAU_SYN[0] * sqrt_Ka[0] / K_FF ;  */
    	J_FF[i] = GAIN_FF * ext_inputs[which_pop[i]] / TAU_SYN[0] * sqrt_Ka[0] / K_FF ; 
    	if(IF_RESCALE) J_FF[i] *= (Vth-Vr[which_pop[i]]) ;
      }
      
      for(i=0; i<N_POISSON; i++)
    	J_FF0[i] = M0_DT + NU_FF * DT ;
	/* J_FF0[i] = 0.0 ; */
	/* J_FF0[i] =  ( 1.0 + var_ff[0] * cos( theta_ff[i] )  ) * M0_DT + NU_FF * DT ;  */
	/* J_FF0[i] =  ( 1.0 + var_ff[0] * cos( theta_ff[i] - 2.0 * white_noise(con_gen) * M_PI)  ) * M0_DT + NU_FF * DT ; */
	/* J_FF0[i] =  ( 1.0 + var_ff[0] *  unif(con_gen) * cos( theta_ff[i] )  ) * M0_DT + NU_FF * DT ; */
	/* J_FF0[i] =  ( 1.0 + ( var_ff[0] + white_noise(con_gen) ) * cos( theta_ff[i] )  ) * M0_DT + NU_FF * DT ; */
	/* J_FF0[i] =  ( 1.0 + var_ff[0] * cos( theta_ff[i] - 2.0 * unif(con_gen) * M_PI )  ) * M0_DT + NU_FF * DT ;  */
	
	/* J_FF0[i] =  ( 1.0 + var_ff[0] * cos( theta_ff[i] - unif(con_gen)) )  * M0_DT + NU_FF * DT ; */
	
	/* J_FF0[i] =  ( 1.0 + sigma_FF[0] * ( white_noise(con_gen) * cos( theta_ff[i] ) + */
	/* 				    white_noise(con_gen) * sin( theta_ff[i] ) )  ) * M0_DT + NU_FF * DT ; */
      
      J_FF0[i] /= GAIN_FF ;
    }
    else {
      
      /* for(i=0; i<n_neurons; i++) {
      /* 	J_FF[i] = ext_inputs[which_pop[i]] / TAU_SYN[0] * sqrt_Ka[0] / N_POISSON ; */
      /* 	  if(IF_NO_MAP)  */
      /* 	    J_FF[i] *= ( 1.0 + var_ff[which_pop[i]]  */
      /* 			 * ( white_noise(con_gen) * cos(theta[i]) */
      /* 			     + white_noise(con_gen) * sin(theta[i]) ) ) ;  */
      /* 	  else */
      /* 	    J_FF[i] *= (1.0 + var_ff[which_pop[i]] * cos(theta[i]) ) ; 	   */
      /* } */
      
      for(i=0; i<n_neurons; i++)
      	for(j=0; j<N_POISSON; j++) {
      	  J_FF_all[j+i*N_POISSON] = ext_inputs[which_pop[i]] / TAU_SYN[0] * sqrt_Ka[0] / N_POISSON ;
	  J_FF_all[j+i*N_POISSON] *= (1.0 + var_ff[which_pop[i]] * cos(theta_ff[j]-theta[i]) ) ; 
      	}
      
      for(i=0; i<n_neurons; i++) 
      	J_FF0[i] = 0.0 ; 
    }
    
    cout << "J_FF0 raw " << J_FF0[0] << " " << J_FF0[1] << endl ; 
    cout << "J_FF raw " << ext_inputs[0] << " scaled " << J_FF[0] << " " ; 
    cout << "J_FF raw " << ext_inputs[1] << " scaled " << J_FF[n_neurons-1] << " " ; 
  }
  
  if(IF_NO_MAP) {
    for(i=0; i<n_neurons; i++)
      bg_inputs[i] *= ( 1.0 + sigma_FF[which_pop[i]]/sqrt_Ka[0]
  			* ( white_noise(con_gen) * cos(theta[i])
  			    + white_noise(con_gen) * sin(theta[i]) )
  			) ;
  }
  
  if(IF_COS_NOISE) { 
    cout << "var_ff " << endl ; 
    if(IF_SQRT_K_NOISE)
      for(i=0;i<n_pop;i++) {
	var_ff[i] = ext_inputs_scaled[i] * sigma_FF[i] ; 
	cout << i << " raw " << sigma_FF[i] << " scaled " << var_ff[i] << " " ; 
      }
    else
      for(i=0;i<n_pop;i++) {
	var_ff[i] = ext_inputs_scaled[i] * sigma_FF[i] / sqrt_Ka[0] ; 
	cout << i << " raw " << sigma_FF[i] << " scaled " << var_ff[i] << " " ; 
      }    
    cout << endl ; 
    
    for(i=0; i<n_neurons; i++) 
      bg_inputs[i] += sqrt_Ka[0] * ext_inputs[which_pop[i]] * NU_FF ; 
  } 
  
  if(IF_TUNED_EXT) { 
    for(i=0; i<n_neurons; i++) { 
      bg_inputs[i] *= ( 1.0 + ext_inputs_scaled[which_pop[i]] * sigma_FF[which_pop[i]] / sqrt_Ka[0] * cos(theta[i]) ) ; 
      bg_inputs[i] += sqrt_Ka[0] * ext_inputs[which_pop[i]] * NU_FF ; 
    } 
  } 
  
}

void init_lif_globals() { 

  duration = DURATION + TIME_STEADY + TIME_WINDOW ; 
  time_rec = TIME_REC + TIME_STEADY + TIME_WINDOW ; 
  time_rec_spikes = TIME_REC_SPIKES + TIME_STEADY ; 
  time_steady = TIME_STEADY ; 
  
  volt = new float [n_neurons]() ; // instantaneous individual membrane voltage 
  t_spike = new float [n_neurons]() ; // time of spike emission 
  if(IF_STP_FF)
    t_spike_ff = new float [N_POISSON]() ; // time of spike emission 
  
  scale_ext_inputs() ; 
  scale_J() ; 
  if(IF_NMDA) scale_J_nmda() ; 
  
  /* if(IF_LOW_RANK) scale_ksi() ; */ 
  
}

void delete_lif_globals() { 
  delete [] volt ; 
  delete [] t_spike ; 
}

void integrate_mem_volt() {

  if(IF_RK2) { 
    /* RK1 = -(volt[i_neuron]-Vl) / TAU_MEM[pre_pop] + net_inputs[i_neuron] ; */
    /* RK2 = -(volt[i_neuron]-Vl + DT*RK1) / TAU_MEM[pre_pop] + net_inputs_RK2[i_neuron] ; */
    /* volt[i_neuron] = volt[i_neuron] + DT/2.0 * ( RK1 + RK2 ) ;  */
    volt[i_neuron] /= one_plus_dt_over_tau_mem[pre_pop] ;
    volt[i_neuron] += net_inputs[i_neuron] * dt_over_dt_tau_mem[pre_pop] ; 
  } 
  else { 
    /* volt[i_neuron] *= one_minus_dt_over_tau_mem[pre_pop] ; */ 
    volt[i_neuron] *= EXP_DT_TAU_MEM[pre_pop] ; 
    volt[i_neuron] += dt_over_tau_mem[pre_pop] * ( net_inputs[i_neuron] + Vl[pre_pop] ) ; // V(t+dt)
    /* volt[i_neuron] += DT * ( net_inputs[i_neuron] + Vl/TAU_MEM[pre_pop] ) ; // V(t+dt) */ 
  } 
} 

void print_rates() {
  
  if( t_window<TIME_WINDOW ) { 
    cout << int(percentage*100.0) << "% " ; 
    cout << "\r" ; 
    cout.flush() ; 
  }
  
  if( t_window>=TIME_WINDOW ) { 
    
    cout << int(percentage*100.0) << "% " ; 
    
    // mean rates 
    cout << " t " << t_time-time_steady << " ms |";
    
    cout << " m0 " << m0 * 1000.0 ; 
    
    if(IF_FF_LAYER) {
      cout << " ff_rates " ;
      cout << ( (float) poisson_rates) * 1000. / TIME_WINDOW / (float) N_POISSON ; 
      poisson_rates = 0 ; 
    }
    
    cout << " rates:"  ;
    for(i=0;i<n_pop;i++) 
      cout << " " << mean_rates[i]*1000./TIME_WINDOW/(float)n_per_pop[i] ; 
    
    if(IF_SPEC || IF_RING || IF_GAUSS) { 
      get_m1_phase() ;
      cout << " m1 " ; 
      for(i=0;i<n_pop;i++)
	cout << m1[i] << " " ;
      
      cout << "phase " ; 
      for(i=0;i<n_pop;i++)
	cout << phase[i] << " " ;
    }
    
    if(IF_LOW_RANK) {
      cout << " | overlaps:" ; 
      for(i=0;i<n_pop;i++) 
	/* cout << " " << overlaps[i] * DT * IS_STRUCT_SYN[i] ;  */
	cout << " " << overlaps[i] * 1000. / TIME_WINDOW / (float) n_per_pop[i] ;
      if(RANK==2) {
	cout << " | map 2:" ; 	
	for(i=0;i<n_pop;i++) {
	  cout << " " << overlaps_1[i] * 1000. / TIME_WINDOW / (float) n_per_pop[i] ;
	  overlaps_1[i] = 0 ; 
	}
      }
      
    }
    
    cout << "\r" ; 
    cout.flush() ; 
    /* cout << endl ;  */ 
  } 
  
}


void update_postsyn_currents_LR() { 
  
  j=idx_post[i_neuron] ;
  post_pop = which_pop[id_post[j]] ;
  
  if(pre_pop==0) { // only E to E/I 
    
    if(IF_STP) {
      while(post_pop==0) { // only EtoE
	inputs[pre_pop][id_post[j]] += A_u_x_stp[i_neuron] * ksi_scaled[i_neuron+id_post[j]*n_per_pop[0]] ; 
	j++ ; 
	post_pop = which_pop[id_post[j]] ; 
      }
      
      while(j<idx_post[i_neuron] + n_post[i_neuron]) { // EtoI 
	inputs[pre_pop][id_post[j]] += J_scaled[pre_pop + post_pop * n_pop] ; 
	j++ ; 
      } 
      
    }// end STP 
    else { // no STP       
      while(post_pop==0) { // only EtoE
	inputs[pre_pop][id_post[j]] += ksi_scaled[i_neuron+id_post[j]*n_per_pop[0]] ; 
	j++ ; 
	post_pop = which_pop[id_post[j]] ; 
      }
      
      while(j<idx_post[i_neuron] + n_post[i_neuron]) { // EtoI 
	inputs[pre_pop][id_post[j]] += J_scaled[pre_pop + post_pop * n_pop] ; 
	j++ ; 
      } 
      
    } // end no STP
  }
  else 
    for(j=idx_post[i_neuron]; j<idx_post[i_neuron] + n_post[i_neuron]; j++) { 
      post_pop = which_pop[id_post[j]] ; 
      inputs[pre_pop][id_post[j]] += J_scaled[pre_pop + post_pop * n_pop] ; 
    } 
} 

void update_postsyn_currents_ff() { 
  
  for(j=idx_post_ff[i_neuron_ff]; j<idx_post_ff[i_neuron_ff] + n_post_ff[i_neuron_ff]; j++) { 
    ff_inputs[id_post_ff[j]] += J_FF[id_post_ff[j]] ; 
    if(IF_STP_FF) 
      ff_inputs[id_post_ff[j]] += J_FF[id_post_ff[j]] * A_u_x_stp_ff[i_neuron_ff]; 
    else
      ff_inputs[id_post_ff[j]] += J_FF[id_post_ff[j]] ; 
  }
  
  /* ff_inputs[id_post_ff[j]] += J_FF_all[i_neuron_ff + id_post_ff[j] * N_POISSON] ;  */
  /* ff_inputs[id_post[j]] += J_FF[id_post[j]] * J_FF0[i_neuron_ff] ;  */
} 

void update_postsyn_currents() {
  /* int counter_E=0, counter_I=0 ; */
  /* cout << "pre_pop "<< pre_pop << " i_neuron " << i_neuron << endl ; */
  
  float J_dum ; 
  for(j=idx_post[i_neuron]; j<idx_post[i_neuron] + n_post[i_neuron]; j++) { 
    post_pop = which_pop[id_post[j]] ;
    
    /* cout << "pre_pop "<< pre_pop << " i_neuron " << i_neuron ; */
    /* cout << " post_pop "<< post_pop << " j_neuron " << id_post[j] ;  */
    /* cout << " J " << J_scaled[pre_pop + post_pop * n_pop] ; */
    /* cout << "\r" ; */
    /* cout.flush() ; */
    
    /* if(post_pop==0) */
    /*   counter_E++ ; */
    /* if(post_pop==1) */
    /*   counter_I++ ; */
        
    J_dum = J_scaled[pre_pop + post_pop * n_pop] ; 
    if(IF_STP && stp_synapse[pre_pop + post_pop * n_pop]) 
      J_dum *= A_u_x_stp[i_neuron] ; 
    if(IF_RK2) 
      J_dum *= exp(-(ISI)/TAU_SYN[pre_pop + post_pop * n_pop]) ; 
    inputs[pre_pop][id_post[j]] += J_dum ; 
  }
  
  /* cout << "n post " << counter_E << " " << counter_I << endl ;  */
  
} 

void update_postsyn_currents_nmda() {
  float J_dum ; 
  if(pre_pop==0)
    for(j=idx_post[i_neuron]; j<idx_post[i_neuron] + n_post[i_neuron]; j++) { 
      post_pop = which_pop[id_post[j]] ; 
      J_dum = J_nmda[pre_pop + post_pop * n_pop] ; 
      if(IF_STP && stp_synapse[pre_pop + post_pop * n_pop]) 
	J_dum *= A_u_x_stp[i_neuron] ; 
      if(IF_RK2) 
	J_dum *= exp(-(ISI)/TAU_NMDA[pre_pop+post_pop*n_pop]) ; 
      inputs_nmda[pre_pop][id_post[j]] += J_dum ; 
    } 
} 

void update_sparse_ff() { 
  for(i_neuron_ff=0; i_neuron_ff<N_POISSON; i_neuron_ff++) 
    if(unif(rand_gen) < J_FF0[i_neuron_ff] ) { 
      /* if(unif(rand_gen) < M0_DT) { */
      poisson_rates ++ ; 
      update_postsyn_currents_ff() ;
      
      if(IF_STP_FF) {
	ISI_FF = t_time - t_spike_ff[i_neuron_ff] ; 
	t_spike_ff[i_neuron_ff] = t_time ;
	mato_ff() ; 
      }
    }
  
  if(IF_FF_EI==0) // poisson only to E
    for(i=n_per_pop[0];i<n_per_pop[1];i++) 
      ff_inputs[i] = ext_inputs_scaled[1] ; 
} 

void update_full_ff() {
  
  for(i_neuron_ff=0; i_neuron_ff<N_POISSON; i_neuron_ff++) 
    if(unif(rand_gen) <  M0_DT ) { 
      poisson_rates ++ ; 
      for(i=0; i<n_neurons; i++) 
	ff_inputs[i] += J_FF_all[i_neuron_ff+i*N_POISSON] ;
      /* ff_inputs[i] += J_FF[i] ;  */
    } 
  
  if(IF_FF_EI==0) // poisson only to E 
    for(i=n_per_pop[0];i<n_per_pop[1];i++) 
      ff_inputs[i] = ext_inputs_scaled[1] ; 
}

void update_noise() {
  
  if(IF_NO_MAP) { 
    for(i=0;i<n_pop;i++)  
      if(var_ff[i]>0.0)  
	for(j=cum_n_per_pop[i]; j<cum_n_per_pop[i+1]; j++) {
	  ff_inputs[j] += var_ff[i] * white_noise(rand_gen) * cos( theta[j] ) ; 
	  ff_inputs[j] += var_ff[i] * white_noise(rand_gen) * sin( theta[j] ) ;
	}
  } 

  if(IF_COS_NOISE){ 
    if(var_ff[0]>0.0) 
      noisy_phase = unif(rand_gen) ; 
    
    for(i=0;i<n_pop;i++) 
      if(var_ff[i]>0.0) 
	for(j=cum_n_per_pop[i]; j<cum_n_per_pop[i+1]; j++) 
	  ff_inputs[j] += var_ff[i] * cos( theta[j] - 2.0 * noisy_phase * M_PI) ; 
  }
  
  if(IF_GAUSS_NOISE) 
    for(i=0;i<n_pop;i++) 
      if(var_ff[i]>0.0) 
	for(j=cum_n_per_pop[i]; j<cum_n_per_pop[i+1]; j++) 
	  ff_inputs[j] += var_ff[i] * white_noise(rand_gen) ; 
  
}

void update_net_inputs() { 
  /* cout << "reseting net inputs" << endl ; */
  
  // Feedforward inputs
  if(IF_FF_LAYER)
    if(IF_SPARSE_FF)
      update_sparse_ff() ; 
    else
      update_full_ff() ; 
  else { 
    for(i=0;i<n_neurons;i++) 
      ff_inputs[i] = bg_inputs[i] ;
    
    if(IF_NOISE) update_noise() ; 
  }
  
  for(i=0;i<n_neurons;i++) { 
    net_inputs[i] = ff_inputs[i] ; 
    
    if(IF_FF_LAYER) 
      net_inputs[i] += J_task[i] ; 
    
    if(REC_FF) filter_ff_inputs[i] += ff_inputs[i] ; 
    if(IF_FF_LAYER && IF_SYN_DYN_FF) ff_inputs[i] *= EXP_DT_TAU_SYN[0] ; 
  }
  
  if(IF_RK2) 
    for(i=0;i<n_neurons;i++) 
      net_inputs_RK2[i] = ff_inputs[i] ; 
  
  // Recurrent inputs  
  for(i=0;i<n_pop;i++)  // presynaptic pop 
    for(j=0;j<n_neurons;j++) { // postsynaptic neuron 
      post_pop = which_pop[j] ; 
      net_inputs[j] += inputs[i][j] ; // must be before decay 

      if(REC_INPUTS) filter_inputs[i][j] += inputs[i][j] ; 
      if(IF_SYN_DYN) inputs[i][j] *= EXP_DT_TAU_SYN[i+post_pop*n_pop] ; 
      
      if(IF_RK2) net_inputs_RK2[j] += inputs[i][j] * EXP_DT_TAU_SYN[i+post_pop*n_pop] ; 
      
      if(IF_NMDA) { 
	net_inputs[j] += inputs_nmda[i][j] ; // must be before decay
	
	if(REC_INPUTS) filter_inputs[i][j] += inputs_nmda[i][j] ; 
	inputs_nmda[i][j] *= EXP_DT_TAU_NMDA[i+post_pop*n_pop] ; 
	
	if(IF_RK2) net_inputs_RK2[j] += inputs_nmda[i][j] * EXP_DT_TAU_NMDA[i+post_pop*n_pop] ; 
      } 
      if(!IF_SYN_DYN) inputs[i][j] = 0.0 ; // delta synapses 
    } 
} 

void initial_conditions() { 
    
  if(IF_STP) { 
    for(i=0;i<n_neurons;i++) { 
      x_stp[i] = 1.0 ;
      u_stp[i] = USE[which_pop[i]] ;
    } 
  } 
  
  if(IF_STP_FF) {
    for(i=0;i<N_POISSON;i++) { 
      x_stp_ff[i] = 1.0 ;
      u_stp_ff[i] = USE_FF ;
    }
  }
  
  /* float mean_V[2] = { -unif(rand_gen) * (Vth-Vr[pre_pop]) + Vth , -unif(rand_gen) * (Vth-Vr[pre_pop]) + Vth}; */
  /* float sigma_V[2] = { unif(rand_gen) * (Vth-Vr[pre_pop]), unif(rand_gen) * (Vth-Vr[pre_pop]) } ; */
  
  for(i_neuron=0; i_neuron<n_neurons; i_neuron++) { 
    pre_pop = which_pop[i_neuron] ; 
    /* volt[i_neuron] = mean_V[pre_pop] + sqrt(sigma_V[pre_pop]) * white_noise(rand_gen) ; */ 
    volt[i_neuron] = unif(rand_gen) * (Vth-Vr[pre_pop]) + Vr[pre_pop] ; 
    /* volt[i_neuron] = mean_V[pre_pop] + sqrt(sigma_V[pre_pop]) * white_noise(rand_gen) ; */
  }
}

void run_sim_lif() { 

  open_files() ;
  
  init_lif_globals() ; 
  open_lif_files() ; 

  if(IF_STP) {
    open_stp_files() ;
    init_stp_globals() ;
  }
    
  initial_conditions() ; 
  
  print_sim_info() ; 
  
  cout << fixed << setprecision(2) ;
  
  for(t_time=0.; t_time<=duration; t_time+=DT) { 
    
    percentage = t_time/duration ; 

    if(IF_STIM) 
      tasks_inputs() ; 
    if(IF_TRACK) 
      track_input() ; 
    
    for(i_neuron=0; i_neuron<n_neurons; i_neuron++) {
      
      pre_pop = which_pop[i_neuron] ; 
      integrate_mem_volt() ; 
      
      if(volt[i_neuron]>=Vth) { // if spike 
	
	if(IF_RK2) { 
	  /* t_spike[i_neuron] = t_time + DT * (Vth-vold) / (volt[i_neuron]-vold) ;  this is what meunier wrote */ 
	  t_spike[i_neuron] = t_time + DT * (Vth-volt[i_neuron]) / (volt[i_neuron]-vold) ; // this is what hansel is using
	  ISI = t_time - t_spike[i_neuron] ; 
	  volt[i_neuron] = (volt[i_neuron]-Vth) * (one_plus_dt_over_tau_mem[pre_pop] * (vold-Vl[pre_pop]) /(volt[i_neuron]-vold) ) + Vl[pre_pop] ; 
	}
	else { 
	  ISI = t_time - t_spike[i_neuron] ; 
	  t_spike[i_neuron] = t_time ; 
	  volt[i_neuron] = Vr[pre_pop] ; 
	}
	
      	if(t_time>=time_steady) { 
      	  mean_rates[pre_pop] += 1 ; 
      	  filter_rates[i_neuron] += 1 ; 
	  
	  if(IF_LOW_RANK && i_neuron<n_per_pop[0]) {
	    overlaps[pre_pop] += ksi[i_neuron] ; 
	    if(RANK==2) 
	      overlaps_1[pre_pop] += ksi_1[i_neuron] ; 
	  }
	  
      	  if(t_time<time_rec_spikes) // save spike times 
      	    file_spike_times << fixed << setprecision(1) << (float) (i_neuron) 
      	  		     << " " << (float) (t_spike[i_neuron]-time_steady) << endl ; 
      	}
	
	if(IF_STP) 
      	  update_stp_variables_lif() ; 
	
	/* if(IF_LOW_RANK) */
	/*   update_postsyn_currents_LR() ; */
	/* else  */
	update_postsyn_currents() ; 
	if(IF_NMDA) 
	  update_postsyn_currents_nmda() ; 
	
      } //endif spike 
      /* else */
      /* 	release_stp() ; */
      
    } //endfor neurons 
    
    update_net_inputs() ; 
    
    print_rates() ; 
    
    if(t_window>=TIME_WINDOW) {
      
      if(t_time<=time_rec) {
	save_to_file() ;
	if(REC_FF)
	  save_ff_inputs() ; 
      }
            
      if(IF_STP) 
	save_xy_to_file() ; 
      
      t_window=0. ; 
    } 
    
    if(t_time >= time_steady) {
      t_window += DT ; 
      if(t_time<=time_rec && IF_SAVE_VOLT) 
	save_volt() ; 
    }
    
  } //endfor time 
  cout << endl ; 
  
  delete_globals() ; 
  close_files() ; 
  
  delete_lif_globals() ; 
  close_lif_files() ; 
  
  if(IF_STP) {
    delete_stp_globals() ; 
    close_stp_files() ;
  }
}

#endif
