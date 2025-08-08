**CUI//REL TO USA ONLY**

# air_to_air demo

This is a collection of several demos that demonstrate air-to-air combat using
the SA Processor and Advanced Behavior Trees (ABTs).

The following demos are included (all demos use the Brawler mover 
(WSF_BRAWLER_MOVER), unless otherwise indicated): 

* 1v1  
      This is a simple one-versus-one engagement.

* 2v2  
      This is a two-versus-two engagement.

* 2v2_am  
      This is a two-versus-two engagement (uses WsfAirMovers).

* 2v2_six_dof  
      This is a two-versus-two engagement (uses WsfPointMassSixDOF_Movers).

* hvaa_dca  
      This is a HVAA-P/DCA mission.

* escort  
      This is a bomber escort mission.

## Notes

1. observer.txt & observer_utils.txt create the brawler standard output file
(log.# & iout.#) user can turn output off by setting log_print and iout_print
to false in the startup file

1. Most aircraft inherit from LTE_FIGHTER which inherits from BRAWLER_TEST,
which contains the behavior tree and brawler processor.

1. Conventions assumed in this behavior tree:
   - all missiles are of category "missile"
   - all aircraft are of category "aircraft"
   - each aircraft is a part of a command_chain named "IFLITE"
   - each aircraft is a part of a command_chain named "ELEMENT"
   - platforms will have 4 missile objects attached to them, with
      some quantity >= 0

1. Flights & Elements: 
   - Flights & elements are handled with command_chains
   - In ftr_rules, only the element lead will make final decision to change
     phase of each member of the element.

1. Missiles:
   - fox2 uses the SHORT_RANGE_IR_MISSILE included with the brawler demo
     in AFSIM.
   - fox3 uses the MEDIUM_RANGE_RADAR_MISSILE included with the brawler
     demo in AFSIM with tweaks.
       - uses a guided mover instead of a straight line mover
       - new aero data was created
       - launch computer built from weapon_tools

1. Aircraft:
   - Aircraft use the Brawler Mover and Brawler Processor which is where
     the EZJA behavior tree resides
   - Mover utilizes the lte_fighter.fxw that is released w/ the unclass
     Brawler release V8.4
   - Added some dummy IR signatures into that file so the user can use 
     the Brawler_to_SOSM.m matlab conversion script and test the SOSM
     interface.

1. Escort Demo:
   - The escort demo does not work with Linux, due to a problem with the
     WSF_BRAWLER_MOVER.

## CUI Designation Indicator
* Controlled by: Air Force Research Laboratory
* Controlled by: Aerospace Systems Directorate
* CUI Categories: CTI, EXPT
* LDC/Distribution Statement: DIST-C, REL TO USA ONLY
* POC: afrl.rq.afsim@us.af.mil

## Notices and Warnings

### DISTRIBUTION STATEMENT C
Distribution authorized to US Government agencies and their contractors;
Critical Technology, Export Controlled; (2021 Mar 09). Other requests for this
information shall be referred to AFRL Aerospace Systems Directorate.

### WARNING - EXPORT CONTROLLED
This content contains technical data whose export is restricted by the Arms
Export Control Act (Title 22, U.S.C. Sec 2751 et seq.) or the Export
Administration Act of 1979, as amended, Title 50 U.S.C., App. 2401 et seq.
Violations of these export laws are subject to severe criminal penalties.
Disseminate in accordance with provisions of DoD Directive 5230.25.

### HANDLING AND DESTRUCTION NOTICE
Handle this information in accordance with DoDI 5200.48. Destroy by any
approved method that will prevent unauthorized disclosure or reconstruction of
this information in accordance with NIST SP 800-88 and 32 C.F.R 2002.14
(Safeguarding Controlled Unclassified Information).

**CUI//REL TO USA ONLY**
