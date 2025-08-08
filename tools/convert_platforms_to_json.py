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
## This script converts AFSIM platform definitions into JSON files to be used
## in the Satellite Inserter tool. While it converts all platform definitions,
## it only populates information specific to satellites.
##
## Usage: convert_platforms_to_json.py [-h | --help] database [files [files ...]]
##
## Where:
##
## [-h | –help] - An optional command to show help message.
##
## database - The JSON database that will contain the JSON representation of the
##            AFSIM platforms and their file locations.
##
## files - A list of .txt files and/or directories that hold AFSIM platform
##         definitions to be represented in database. Default: “.” - the
##         current directory.
##
###############################################################################

from contextlib import contextmanager
import json
import os
import argparse
import subprocess

def readFile(filePath):
    with open(filePath, 'r') as f:
        platformFound = False
        platform = ""
        platform_type = ""
        definition_type = "nominal"
        orbit_type = ""
        classification = ""
        designator = ""
        constellation = ""
        for line in f:
            stripLine = line.strip().rstrip();
            str = stripLine.split(' ')
            if str[0] == "platform" and platformFound == False:
                platformFound = True
                platform = str[1]
                platform_type = str[2].rstrip()
                if (platform_type == "EGNOS" or
                        platform_type == "WAAS" or
                        platform_type == "MSAS" or
                        platform_type == "QZSS" or
                        platform_type == "GAGAN" or
                        platform_type == "SDCM"):
                    constellation = "SATELLITE_BASED_NAVIGATION_SYSTEM"
                else:
                    constellation = platform_type
            elif str[0] == "orbit":
                definition_type = "tle"
            elif str[0] == "1":
                if str[1].endswith("U"):
                    classification = "unclassified"
                else:
                    classification = "classified"
                designator = str[2].rstrip()
            elif str[0] == "designator":
                designator = str[1].rstrip()
                if designator == platform:
                    designator = ""
            elif str[0].rstrip() == "end_platform":
                data['platforms'].append({
                    'name': platform,
                    'platform_type': platform_type,
                    'designator': designator,
                    'country': "",
                    'definition_type': definition_type,
                    'orbit_type': "",
                    'classification': classification,
                    'constellation': constellation,
                    'norad_catalog_number': "",
                    'launch_date': "",
                    'launch_site': "",
                    'radar_cross_section': "",
                    'operational_status': "",
                    'file': filePath
                })
                platform = ""
                definition_type = "nominal"
                platform_type = ""
                orbit_type = ""
                classification = ""
                designator = ""
                constellation = ""
                platformFound = False


parser = argparse.ArgumentParser()
parser.add_argument('database', nargs=1, help='A database to create. Must be a .json file.')
parser.add_argument('files', nargs='*', default=".", help='One or more files and/or directories to use to create the database.')
args = parser.parse_args()
if not args.database[0].endswith(".json"):
    parser.print_help()
    exit()
data = {}
data['platforms'] = []
with open(args.database[0], 'w') as satellite:
    for file in args.files:
        if os.path.isdir(file):
            for path, dirs, files in os.walk(file):
                for directoryFile in files:
                    if directoryFile.endswith(".txt"):
                        filePath = os.path.join(path, directoryFile)
                        print(filePath)
                        readFile(filePath)
        elif os.path.isfile(file):
            readFile(file)
        else:
            print("***WARNING: %s is not a valid file or directory.\n" % file)

    json.dump(data, satellite, indent=3, separators=(',', ': '))
