/* Name : Coef_Remus.h
 * Author : Nathan Vandervaeren
 * Date : April 2014
 * 
 * This file is used by the class Dynamics. 
 * 
 * It contains the vehicle coefficients for the REMUS AUV. 
 * They come from T.Prestero, "Verification of a six-degree of freedom 
 * simulation model for the Remus autonomous underwater vehicle".
 * 
 * Note : only the adjusted coefficients in the appendix are used here.
 */

#define W +299
#define B +299 // in Prestero, B = 306 N. We assume here B = W

#define m  +30.48

#define xg +0.00 
#define yg +0.00 
#define zg +0.0196

#define Ixx +0.177
#define Iyy +3.45
#define Izz +3.45

// Thrust is varying in our applications
// #define Kprop  -5.43 * pow(10, -001) // Propeller Torque
// #define Xprop  +3.86 * pow(10, +000) // Propeller Thrust

#define Xuu     -1.62   // Cross-flow Drag
#define Xudot   -0.93   // Added Mass
#define Xwq     -35.5   // Added Mass Cross-term
#define Xqq     -1.93   // Added Mass Cross-term
#define Xvr     +35.5   // Added Mass Cross-term
#define Xrr     -1.93   // Added Mass Cross-term
#define Yvv     -1310.  // Cross-flow Drag
#define Yrr     +0.632  // Cross-flow Drag
#define Yuv     -28.6   // Body Lift Force and Fin Lift
#define Yvdot   -35.5   // Added Mass
#define Yrdot   +1.93   // Added Mass
#define Yur     +5.22   // Added Mass Cross-term and Fin Lift
#define Ywp     +35.5   // Added Mass Cross-term
#define Ypq     +1.93   // Added Mass Cross-term
#define Yuudr   +9.64   // Fin Lift Force
#define Zww     -131.   // Cross-flow Drag
#define Zqq     -0.632  // Cross-flow Drag
#define Zuw     -28.6   // Body Lift Force and Fin Lift
#define Zwdot   -35.5   // Added Mass
#define Zqdot   -1.93   // Added Mass
#define Zuq     -5.22   // Added Mass Cross-term and Fin Lift
#define Zvp     -35.5   // Added Mass Cross-term
#define Zrp     +1.93   // Added Mass Cross-term
#define Zuuds   -9.64   // Fin Lift Force
#define Kpp     -0.130  // Rolling Resistance
#define Kpdot   -0.0704 // Added Mass
#define Mww     +3.18   // Cross-flow Drag
#define Mqq     -188.   // Cross-flow Drag
#define Muw     +24.0   // Body and Fin Lift and Munk Moment
#define Mwdot   -1.93   // Added Mass
#define Mqdot   -4.88   // Added Mass
#define Muq     -2.00   // Added Mass Cross-term and Fin Lift
#define Mvp     -1.93   // Added Mass Cross-term
#define Mrp     +4.86   // Added Mass Cross-term
#define Muuds   -6.15   // Fin Lift Moment
#define Nvv     -3.18   // Cross-flow Drag
#define Nrr     -94.0   // Cross-flow Drag
#define Nuv     -24.0   // Body and Fin Lift and Munk Moment
#define Nvdot   +1.93   // Added Mass
#define Nrdot   -4.88   // Added Mass
#define Nur     -2.00   // Added Mass Cross-term and Fin Lift
#define Nwp     -1.93   // Added Mass Cross-term
#define Npq     -4.86   // Added Mass Cross-term
#define Nuudr   -6.15   // Fin Lift Moment


