# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************

###############################################################################
##
## make_satcat_database.py is a python script that converts raw SATCAT 
## satellite definitions to both AFSIM platform definitions and a JSON database
## to be used in the Satellite Inserter tool. It only creates definitions for
## non/operational satellites.
##
## Usage:
##
## make_satcat_database.py [-h | –help] data new_database new_definitions_file
##
## Where:
##
## [-h | –help] - An optional command to show help message.
##
## data - The raw SATCAT data. The most up-to-date SATCAT data can be found at SATCAT.
##
## database - The JSON database that will contain the JSON representation of the SATCAT satellites and their file locations.
##
## new_definitions_file - A .txt file that holds the SATCAT satellites’ AFSIM platform definitions.
###############################################################################

import json
import os
import argparse


parser = argparse.ArgumentParser()
parser.add_argument('data', nargs=1, help="The raw SATCAT data.")
parser.add_argument('new_database', nargs=1, help='The new SATCAT database file. Must be a .json file.')
parser.add_argument('new_definitions_file', nargs=1, help='The file which contains the new SATCAT definitions.')
args = parser.parse_args()

if not args.new_database[0].endswith(".json"):
    parser.print_help()
    exit()

satcat_data = {}
satcat_data['platforms'] = []
satcat_json_only = {}
satcat_json_only['platforms'] = []

with open(args.data[0], 'r') as raw:
    for line in raw:
        if line[21] != '+' and line[21] != '-' and line[21] != 'U':
            continue
        if line[21] == '+' or line[21] == 'U':
            operational_status = 'operational'
        else:
            operational_status = 'non_operational'
        platform_type = "WSF_PLATFORM"
        country = line[49:53].strip()
        name = line[23:47]
        if name.find('['):
            start = name.find('[')
            name = name[:start]
        if name.find('('):
            start = name.find('(')
            name = name[:start]
        name = name.strip().replace(' ', '_').replace('+', "").replace('.','_')
        if name.endswith("R/B") or name.endswith("DEB"):
            continue
        elif name.endswith("PLAT"):
            name = name[:-5]
        designator = line[0:10].strip().replace("-", "")
        launch_date = line[56:66].strip()
        launch_site = line[68:73].strip()
        radar_cross_section = line[119:126].strip()
        norad_catalog_number = line[13:17]
        inclination = line[96:101].strip()
        if inclination != "":
            inclination = float(inclination)
        else:
            continue
        min_per_rev = line[87:94].strip()
        if min_per_rev != "":
            revs_per_day = 1440.00/float(min_per_rev)
        else:
            continue

        apogee = line[103:109].strip()
        perigee = line[111:117].strip()
        if apogee != "" and perigee != "":
            apogee = float(apogee)
            perigee = float(perigee)
            eccentricity = (apogee - perigee)/(apogee + perigee)

            if eccentricity > 1:
                print("%s, %s, %s, %s" % (name, apogee, perigee, eccentricity))
            
            if apogee <= 2000:
                orbit_type = "LEO"
            elif perigee > 2000 and apogee < 35686:
                orbit_type = "MEO"
            elif eccentricity >= .4:
                orbit_type = "HEO"
            elif eccentricity < .001 and inclination < 90 and apogee >= 35686 and apogee <= 35886:
               orbit_type = "GEO"
            else:
                orbit_type = "" 
        else:
            eccentricity = 0
        if eccentricity < 0.01:
            eccentricity = 0

        satcat_data['platforms'].append({
            'name': name,
            'platform_type': platform_type,
            'designator': designator,
            'revs_per_day': revs_per_day,
            'inclination': inclination,
            'eccentricity ': eccentricity,
            'radar_cross_section': radar_cross_section,
        })

        satcat_json_only['platforms'].append({
            'name': name,
            'platform_type': platform_type,
            'designator': designator,
            'country': country,
            'definition_type': "nominal",
            'orbit_type': orbit_type,
            'constellation': "",
            'classification': "unclassified",
            'norad_catalog_number': norad_catalog_number,
            'launch_date': launch_date,
            'launch_site': launch_site,
            'radar_cross_section': radar_cross_section,
            'operational_status': operational_status,
            'file': args.new_definitions_file[0]
        })

with open(args.new_database[0], 'w') as new_sats:
    json.dump(satcat_json_only, new_sats, indent=3, separators=(',', ': '))
with open(args.new_definitions_file[0], 'w') as new_afsim:
    for plat in satcat_data['platforms']:
        if plat['radar_cross_section'] != 'N/A' and plat['radar_cross_section'] != '':
            new_afsim.write('radar_signature %s_RADAR_SIGNATURE WSF_RADAR_SIGNATURE\n' % plat['name'].upper())
            new_afsim.write('   constant %s m^2\nend_radar_signature\n\n' % plat['radar_cross_section'])
        new_afsim.write('platform %s %s\n' % (plat['name'], plat['platform_type']))
        if plat['platform_type'] == "WSF_PLATFORM":
            new_afsim.write('   add mover WSF_SPACE_MOVER\n')
        else:
            new_afsim.write('   edit mover\n')
        new_afsim.write('      designator %s\n' % plat['designator'])
        new_afsim.write('      revs_per_day %s\n' % plat['revs_per_day'])
        new_afsim.write('      inclination %s deg\n' % plat['inclination'])
        new_afsim.write('      eccentricity  %s\n' % plat['eccentricity '])
        # new_afsim.write('raan %s deg' % plat['raan'])
        # new_afsim.write('anomaly %s deg' % plat['anomaly'])
        new_afsim.write('   end_mover\n')
        if plat['radar_cross_section'] != 'N/A' and plat['radar_cross_section'] != '':
            new_afsim.write('   radar_signature %s_RADAR_SIGNATURE\n' % plat['name'].upper())
        new_afsim.write('end_platform\n\n')
