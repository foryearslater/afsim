#!/usr/bin/env python3

# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# Copyright (C) 2022 Stellar Science; U.S. Government has Unlimited Rights.
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************

# This is pyrunplotter, a tool for plotting comparative graphs for multiple
# multiresolution runs. Refer to the documentation for detailed usage instructions

has_plt = True
has_np = True

import os
import re

# While being imported during documentation build, the following modules may not be available.
# Conditionally import them so that the module loads, and check later that the modules were
# imported if necessary
# matplotlib
try:
    import matplotlib.pyplot as plt
except ImportError:
    has_plt = False

try:
    import numpy as np
except ImportError:
    has_np = False

import pymystic
import math
from pprint import pprint

# Earth ellipsoid parameters taken from AFSIM
earthEllipsoidWGS84 = {
    'semiMajorAxis': 6378137.0,
    'flatteningReciprocal': 298.257223563,
    'semiMinorAxis': 6356752.3142,
    'meanRadius': 6371000.7900,
    'firstEccentricitySquared': 6.69437999014e-3,
    'secondEccentricitySquared': 6.73949674228e-3,
    'rotationRate': 7.2921158553e-5,
    'gravitationalParameter': 3.986004418e+14
}

# Color codes taken directly from AFSIM
afsimColors = {
    'black':        '#000000',
    'blue':         '#00a8dc',
    'brown':        '#3d2100',
    'dark_blue':    '#006b8c',
    'dark_green':   '#00a000',
    'dark_purple':  '#500050',
    'dark_red':     '#c80000',
    'dark_yellow':  '#e1dc00',
    'gray':         '#666666',
    'grey':         '#666666',
    'green':        '#00e200',
    'indigo':       '#4a009f',
    'light_blue':   '#80e0ff',
    'light_green':  '#aaffaa',
    'light_purple': '#ffa1ff',
    'light_red':    '#ff8080',
    'light_yellow': '#ffff80',
    'magenta':      '#ff00ff',
    'orange':       '#ffaa00',
    'pink':         '#ff00c0',
    'purple':       '#800080',
    'red':          '#ff3031',
    'tan':          '#b68538',
    'violet':       '#c080ff',
    'white':        '#ffffff',
    'yellow':       '#ffff00',
}

lineStyles = (
    (0, ()),
    (0, (5, 1)),
    (0, (3, 1)),
    (0, (1, 1)),
    (0, (3, 1, 1, 1)),
    (0, (5, 1, 1, 1)),
    (0, (5, 1, 3, 1)),
    (0, (3, 1, 1, 1, 1, 1)),
    (0, (3, 1, 3, 1, 1, 1)),
    (0, (5, 1, 1, 1, 1, 1)),
    (0, (5, 1, 5, 1, 1, 1)),
    (0, (3, 1, 3, 1, 1, 1, 1, 1)),
    (0, (3, 1, 1, 1, 1, 1, 1, 1)),
    (0, (5, 1, 1, 1, 1, 1, 1, 1)),
    (0, (5, 1, 1, 1, 3, 1, 1, 1)),
    (0, (5, 1, 5, 1, 1, 1, 1, 1)),
)

def convertLlaToWcs(lat, long, alt):
    """
    Adapted from EllipsoidalCentralBody::ConvertLLAToECEF in UtEllipsoidalCentralBody.cpp
    """
    radLat = math.radians(lat)
    radLong = math.radians(long)
    sinLat = math.sin(radLat)
    cosLat = math.cos(radLat)
    sinLong = math.sin(radLong)
    cosLong = math.cos(radLong)
    R_N = earthEllipsoidWGS84['semiMajorAxis'] / math.sqrt(1.0 - earthEllipsoidWGS84['firstEccentricitySquared'] * sinLat * sinLat)
    temp1 = (R_N + alt) * cosLat
    x = temp1 * cosLong
    y = temp1 * sinLong
    z = ((1.0 - earthEllipsoidWGS84['firstEccentricitySquared']) * R_N + alt) * sinLat
    return (x, y, z)

def convertWcsToLla(x, y, z):
    """
    Adapted from EllipsoidalCentralBody::ConvertECEFToLLA in UtEllipsoidalCentralBody.cpp
    """
    p = math.sqrt(x*x + y*y)
    if p > 1.0e-8:
        psi = math.atan2(z * earthEllipsoidWGS84['semiMajorAxis'], p * earthEllipsoidWGS84['semiMinorAxis'])
        sinPsi = math.sin(psi)
        cosPsi = math.cos(psi)
        phi = math.atan2(
            (z + earthEllipsoidWGS84['secondEccentricitySquared'] * earthEllipsoidWGS84['semiMinorAxis'] * sinPsi * sinPsi * sinPsi),
            (p - earthEllipsoidWGS84['firstEccentricitySquared'] * earthEllipsoidWGS84['semiMajorAxis'] * cosPsi * cosPsi * cosPsi))
        theta = math.atan2(y, x)
        cosPhi = math.cos(phi)
        sinPhi = math.sin(phi)
        R_N = earthEllipsoidWGS84['semiMajorAxis'] / math.sqrt(1.0 - earthEllipsoidWGS84['firstEccentricitySquared'] * sinPhi * sinPhi)
        alt = (p / cosPhi) - R_N
        long = math.degrees(theta)
        lat = math.degrees(phi)

    else:
        if (z >= 0.0):
            lat = 90.0
        else:
            lat = -90.0

        long = 0.0
        alt = math.abs(z) - earthEllipsoidWGS84['semiMinorAxis']

    return (lat, long, alt)

def interpolate(array, t, startIndex=0):
    """
    Interpolates values in [array] based on [t]

    Parameters
    ----------
    array : Array of arrays (list-like object of list-like objects)
        The base array of values used to interpolate. For each entry in the array, index [0] should be time and all following indices should be the values to interpolate

    t : real number
        The time to interpolate at

    startIndex : (optional) positive int
        The index (not time value) to start at when searching for the value to interpolate.
    -------
    Returns
    -------
    (interpolatedValue, index)

    interpolatedValue : array (list-like object)
        The interpolated value. Formatted the same as each entry of [array], with index 0 matching [t] and the rest of the indices all being interpolated values

    index : positive int
        The index of the end point of the interpolation. This can be passed in again on the next cycle to save traversal time.
    -------
    """
    a0 = []
    a1 = []
    index = startIndex
    for i, a in enumerate(array[startIndex:]):
        index = startIndex + i
        if a[0] >= t:
            a1 = a
            if index > 0:
                a0 = array[index-1]
            else:
                a0 = a
            break

    if not a0:
        a1 = array[-1]
        for a in array[::-1]:
            if a[0] < a1[0]:
                a0 = a
                break

    if a1[0] != a0[0]:
        tscale = (t - a0[0]) / (a1[0] - a0[0])
    else:
        tscale = 1.0

    aout = [i + tscale*(j - i) for i, j in zip(a0, a1)]

    return (aout, index)

def makeDeltaArray(arraybase, arraycomp, relative=False):
    """
    Interpolates values in [array] based on [t]

    Parameters
    ----------
    arraybase : Array of arrays (list-like object of list-like objects)
        The base array of values used for comparison

    arraycomp : Array of arrays (list-like object of list-like objects)
        The array that the base array will be compared against. Each element must have the same dimensions as the elements of [arraybase]

    relative : (optional) boolean
        If true, each [base] and [comp] value are compared using abs( ([comp] - [base]) / [base] )
        If false, each [base] and [comp] value are compared using abs( [comp] - [base] )
    -------
    Returns
    -------
    deltaarray : Array of arrays (list-like object of list-like objects)
        Array using t-values of [arraybase], showing differences with [arraycomp] at each recorded t-value. Values from [arraycomp] are all interpolated using the above [interpolate()] function
    -------
    """
    arrayout = []
    startIndex = 0
    if len(arraybase[0]) != len(arraycomp[0]):
        raise Exception("Values of arraybase do not match the dimensions of values of arraycomp. Unable to create delta array")

    for base in arraybase:
        t = base[0]
        (comp, startIndex) = interpolate(arraycomp, t, startIndex)
        if relative:
            delta = [t]
            divisionByZero = False
            for b, c in zip(base[1:], comp[1:]):
                if b != 0:
                    delta += [abs((c - b)/b)]
                else:
                    divisionByZero = True
                    break

            if not divisionByZero: # If a division by 0 would occur on any value in [base], skip adding that entry to the output array
                arrayout.append(delta)

        else:
            delta = [t] + [abs(c - b) for b, c in zip(base[1:], comp[1:])]
            arrayout.append(delta)

    return arrayout

class FileProcessor():
    """
    This class is responsible for processing each individual file and collating the data into dictionaries for easy plotting
    """
    def __init__(self, fileName):
        self.fileName = fileName
        self.platformInfo = dict()
        self.platformIndices = dict()
        self.sensorDetectInfo = dict()
        self.weaponInfo = dict()
        self.platformTotalNo = 0
        self.platformNos = dict([(k, 0) for k in afsimColors.keys()])
        self.platformTotalCount = [[0, 0]]
        self.platformCounts = dict([(k, [[0, 0]]) for k in afsimColors.keys()])

    # processFile
    # Reads an .aer file and stores select data in member variables. Designed to process the file in one pass
    def processFile(self, endTime=None, verbose=True, timestep=100):
        with pymystic.Reader(self.fileName) as r:
            a = r.read()
            t = 0
            while (a):
                if (verbose and a['simTime'] >= t):
                    t += timestep
                    print("simTime = {}".format(a['simTime']))

                if (endTime != None and a['simTime'] >= endTime): break

                if (a['msgtype'] == 'MsgPlatformInfo'):
                    self._msgPlatformInfo(a)
                elif (a['msgtype'] == 'MsgPartStatus'):
                    self._msgPartStatus(a)
                elif (a['msgtype'] == 'MsgEntityState'):
                    self._msgEntityState(a)
                elif (a['msgtype'] == 'MsgSensorTrackUpdate'):
                    self._msgSensorTrackUpdate(a)
                elif (a['msgtype'] == 'MsgDetectAttempt'):
                    self._msgDetectAttempt(a)
                elif (a['msgtype'] == 'MsgPlatformStatus'):
                    self._msgPlatformStatus(a)
                elif (a['msgtype'] == 'MsgWeaponQuantityChange'):
                    self._msgWeaponQuantityChange(a)
                elif (a['msgtype'] == 'MsgRouteChanged'):
                    self._msgRouteChanged(a)

                a = r.read()

    # _msgPlatformInfo
    # Handle messages of the type 'MsgPlatformInfo'. This message appears whenever a new platform is initialized.
    # This method initializes a new platform in self.platformInfo and records the platform's creation time in self.platformTotalCount and self.platformCounts
    def _msgPlatformInfo(self, a):
        t = a['simTime']
        self.platformInfo[a['platformIndex']] = {
            'name': a['name'],
            'color': a['side'],
            'locationWCS': [],
            'locationLLA': [],
            'velocityWCS': [],
            'fuel': [],
            'components': [],
            'addedTime': t}

        if self.platformInfo[a['platformIndex']]['color'] == '':
            self.platformInfo[a['platformIndex']]['color'] = 'gray'

        self.platformIndices[a['name']] = a['platformIndex']
        self.platformTotalCount.append([t, self.platformTotalNo])
        self.platformTotalNo += 1
        self.platformTotalCount.append([t, self.platformTotalNo])

        for color in self.platformCounts:
            self.platformCounts[color].append([t, self.platformNos[color]])

        self.platformNos[self.platformInfo[a['platformIndex']]['color']] += 1
        for color in self.platformCounts:
            self.platformCounts[color].append([t, self.platformNos[color]])

    # _msgPartStatus
    # Handle messages of the type 'MsgPartStatus'. This message appears whenever a platform's component is initialized or updated.
    # This method simply records the name and type of the component in a list inside its parent platform's entry in self.platformInfo
    def _msgPartStatus(self, a):
        isDuplicate = False
        for p in self.platformInfo[a['platformIndex']]['components']:
            if (p['partName'] == a['partName'] and p['partType'] == a['partType']):
                isDuplicate = True
                break

        if not isDuplicate:
            self.platformInfo[a['platformIndex']]['components'].append({'partName': a['partName'], 'partType': a['partType']})

    # _msgEntityState
    # Handle messages of the type 'MsgEntityState'. This message appears whenever a platform's state is updated
    # This method records the platform's position, velocity, and fuel level at the given sim time.
    def _msgEntityState(self, a):
        t = a['simTime']
        x = a['state']['locationWCS']['x']
        y = a['state']['locationWCS']['y']
        z = a['state']['locationWCS']['z']
        (lat, long, alt) = convertWcsToLla(x, y, z)
        self.platformInfo[a['state']['platformIndex']]['locationWCS'].append([t, x, y, z])
        self.platformInfo[a['state']['platformIndex']]['locationLLA'].append([t, lat, long, alt])

        if a['state']['velocityWCSValid']:
            vx = a['state']['velocityWCS']['x']
            vy = a['state']['velocityWCS']['y']
            vz = a['state']['velocityWCS']['z']
            self.platformInfo[a['state']['platformIndex']]['velocityWCS'].append([t, vx, vy, vz])

        if a['state']['fuelCurrentValid']:
            fuel = a['state']['fuelCurrent']
            self.platformInfo[a['state']['platformIndex']]['fuel'].append([t, fuel])

    # _msgSensorTrackUpdate
    # Handle messages of the type 'MsgSensorTrackUpdate'. This message appears whenever a sensor's track of a platform changes
    # This method records the sensor's signal-to-noise ratio or its pixel count, depending on which is available. If neither appears in the message, this method doesn't record anything
    def _msgSensorTrackUpdate(self, a):
        t = a['simTime']
        hostPlatform = self.platformInfo[a['ownerIndex']]['name']
        targetPlatform = self.platformInfo[a['track']['targetIndex']]['name']
        sensorName = a['track']['sensorName']
        self._initSensorTarget(hostPlatform, sensorName, targetPlatform)

        if a['track']['signalToNoiseValid']:
            signalToNoise = a['track']['signalToNoise']
            self.sensorDetectInfo[hostPlatform][sensorName]['targets'][targetPlatform]['signalToNoise'].append([t, signalToNoise])

        if a['track']['pixelCountValid']:
            pixelCount = a['track']['pixelCount']
            self.sensorDetectInfo[hostPlatform][sensorName]['targets'][targetPlatform]['pixelCount'].append([t, pixelCount])

    # _msgDetectAttempt
    # Handle messages of the type 'MsgDetectAttempt'. This message appears whenever a sensor attempts to detect a platform
    # This method records the pd, the probability of detection, at the given sim time
    def _msgDetectAttempt(self, a):
        t = a['simTime']
        hostPlatform = self.platformInfo[a['sensorPlatformIndex']]['name']
        targetPlatform = self.platformInfo[a['targetPlatformIndex']]['name']
        sensorName = a['sensorName']
        self._initSensorTarget(hostPlatform, sensorName, targetPlatform)

        pd = a['pd']
        self.sensorDetectInfo[hostPlatform][sensorName]['targets'][targetPlatform]['pd'].append([t, pd])

    # _msgPlatformStatus
    # Handle messages of the type 'MsgPlatformStatus'. This message appears whenever a platform is either damaged to the point of broken, or removed from the sim
    # This method records a platform's time when it gets removed from the simulation
    def _msgPlatformStatus(self, a):
        t = a['simTime']
        if a['broken'] or a['removed']:
            self.platformInfo[a['platformIndex']]['removedTime'] = t

            # To make the plot look nice, record platform counts at this timestamp just before updating the count
            self.platformTotalCount.append([t, self.platformTotalNo])
            for color in self.platformCounts:
                self.platformCounts[color].append([t, self.platformNos[color]])

            self.platformTotalNo -= 1
            self.platformNos[self.platformInfo[a['platformIndex']]['color']] -= 1

            self.platformTotalCount.append([t, self.platformTotalNo])
            for color in self.platformCounts:
                self.platformCounts[color].append([t, self.platformNos[color]])

    # _msgWeaponQuantityChange
    # Handle messages of the type 'MsgWeaponQuantityChange'. This message appears whenever a weapon that uses ammo has its ammo quantity updated
    # This method records a weapon's ammo quantity at the given sim time
    def _msgWeaponQuantityChange(self, a):
        t = a['simTime']
        hostPlatform = self.platformInfo[a['platformIndex']]['name']
        weaponName = a['weaponName']
        self._initWeapon(hostPlatform, weaponName)

        # To make the plot look nice, record ammo quantity at this timestamp just before updating the count
        if len(self.weaponInfo[hostPlatform][weaponName]['quantity']) > 0:
            self.weaponInfo[hostPlatform][weaponName]['quantity'].append([t, self.weaponInfo[hostPlatform][weaponName]['quantity'][-1][1]])

        weaponQuantity = a['weaponQuantity']
        self.weaponInfo[hostPlatform][weaponName]['quantity'].append([t, weaponQuantity])

    # _msgRouteChanged
    # Handle messages of the type 'MsgRouteChanged'. This message appears whenever a platform's route is updated
    # This method stores the route in the specified platform's entry in self.platformInfo
    def _msgRouteChanged(self, a):
        self.platformInfo[a['platformIndex']]['route'] = a['route']

    # _initSensorTarget
    # This method initializes an entry in self.sensorDetectInfo, if the entry does not exist. It initializes a platform, then the platform's sensor, then the sensor's target
    # Once initialized, the entry can store info from e.g. _msgSensorTrackUpdate
    def _initSensorTarget(self, hostPlatform, sensorName, targetPlatform):
        if hostPlatform not in self.sensorDetectInfo:
            self.sensorDetectInfo[hostPlatform] = dict()

        if sensorName not in self.sensorDetectInfo[hostPlatform]:
            self.sensorDetectInfo[hostPlatform][sensorName] = {'targets': dict()}

        if targetPlatform not in self.sensorDetectInfo[hostPlatform][sensorName]['targets']:
            self.sensorDetectInfo[hostPlatform][sensorName]['targets'][targetPlatform] = {
                'pd': [],
                'signalToNoise': [],
                'pixelCount': []}

    # _initWeapon
    # This method initializes an entry in self.weaponInfo, if the entry does not exist. It initializes a platform, then the platform's weapon.
    # Once initialized, the entry can store info from e.g. _msgWeaponQuantityChange
    def _initWeapon(self, hostPlatform, weaponName):
        if hostPlatform not in self.weaponInfo:
            self.weaponInfo[hostPlatform] = dict()

        if weaponName not in self.weaponInfo[hostPlatform]:
            self.weaponInfo[hostPlatform][weaponName] = {
                'quantity': []}

class Analyzer():
    """
    This class is responsible for taking in a file or list of files and plotting data based on the file contents
    """
    def __init__(self, filePattern, endTime=None):
        self.endTime=endTime
        self.filePattern = filePattern
        self.fileProcessors = dict()

    def __enter__(self):
        if not has_np:
            raise ImportError("'numpy' must be installed to use this script")

        if not has_plt:
            raise ImportError("'matplotlib' must be installed to use this script")

        self.processFile(endTime=self.endTime)
        return self
    def __exit__(self, type, value, traceback):
        return None

    def _processMultiFile(self, endTime=None):
        (dirName, filePattern) = os.path.split(self.filePattern)
        if not dirName:
            dirName = '.'

        filePattern = re.escape(filePattern)
        filePattern = filePattern.replace("%d", "(\\d+)")
        fileList = [os.path.join(dirName, f) for f in os.listdir(dirName) if re.fullmatch(filePattern, f)]
        if not fileList:
            raise FileNotFoundError("No files found matching pattern '{}'".format(self.filePattern))

        for f in fileList:
            m = re.fullmatch(filePattern, os.path.split(f)[1])
            num = int(m.group(1))
            try:
                self._processFile(f, runNumber=num, endTime=endTime)
            except FileNotFoundError:
                print(f"Error: '{f}' not found")

    def _processFile(self, fileName, runNumber=1, endTime=None):
        processor = FileProcessor(fileName)
        processor.processFile(endTime=endTime)
        self.fileProcessors[runNumber] = processor

    def processFile(self, endTime=None):
        """
        Called to process either a single file or a group of files, depending on how Analyzer is initialized
        """
        if "%d" in self.filePattern:
            self._processMultiFile(endTime=endTime)
        else:
            self._processFile(self.filePattern, endTime=endTime)

    def _getPlatformInfo(self, platformName, runNumber):
        try:
            proc = self.fileProcessors[runNumber]
        except KeyError:
            print(f"No run with the index [{runNumber}] has been processed")
            print("Valid run numbers:")
            pprint(list(self.fileProcessors.keys()))
            print("------")

        try:
            platformIndex = proc.platformIndices[platformName]
        except KeyError:
            print(f"No platform with name [{platformName}] found in run number [{runNumber}]")
            print("Valid platform(s):")
            pprint(list(proc.platformIndices.keys()))
            print("------")

        return proc.platformInfo[platformIndex]

    def _getSensorDetectInfo(self, sensorPlatform, sensorName, targetPlatform, runNumber):
        try:
            proc = self.fileProcessors[runNumber]
        except KeyError:
            print(f"No run with the index [{runNumber}] has been processed")
            print("Valid run numbers:")
            pprint(list(self.fileProcessors.keys()))
            print("------")

        try:
            sensorDetectInfo = proc.sensorDetectInfo[sensorPlatform]
        except KeyError:
            print(f"No platform with name [{sensorPlatform}] found in run number [{runNumber}]")
            print("Valid platform(s):")
            pprint(list(proc.sensorDetectInfo.keys()))
            print("------")

        try:
            sensorInfo = sensorDetectInfo[sensorName]
        except KeyError:
            print(f"No sensor named [{sensorName}] found on platform [{sensorPlatform}] in run number [{runNumber}]")
            print("Valid sensor(s):")
            pprint(list(sensorDetectInfo.keys()))
            print("------")

        try:
            targetInfo = sensorInfo['targets'][targetPlatform]
        except KeyError:
            print(f"No sensor detection info found for target [{targetPlatform}] from sensor [{sensorName}] on platform [{sensorPlatform}] in run number [{runNumber}]")
            print("Valid target(s):")
            pprint(list(sensorInfo['targets'].keys()))
            print("------")

        return targetInfo
    def plotPosition3D(self, platformList, plotWaypoints=True, runNumberList=[1]):
        """
        Takes a list of platforms and, optionally, a list of run numbers, and plots each platform from each run in a 3D position plot

        """
        ax = plt.axes(projection = '3d')
        for i, n in enumerate(runNumberList):
            for p in platformList:
                platformInfo = self._getPlatformInfo(p, n)
                positionVsTime = np.array(platformInfo['locationWCS'])
                if (len(runNumberList) == 1):
                    labelStr = p
                else:
                    labelStr = f"{p}:{n}"

                color = afsimColors[platformInfo['color']]
                ax.plot3D(positionVsTime[:,1], positionVsTime[:,2], positionVsTime[:,3], label=labelStr, color=color, linestyle=lineStyles[i%len(lineStyles)])
                if plotWaypoints:
                    waypoints = []
                    if 'route' in platformInfo:
                        for waypoint in platformInfo['route']:
                            lat = waypoint['locationX']
                            long = waypoint['locationY']
                            if 'altitude' in waypoint:
                                alt = waypoint['altitude']
                            else:
                                alt = 0
                            (x, y, z) = convertLlaToWcs(lat, long, alt)
                            waypoints.append([x, y, z])

                    else:
                        waypoints.append([positionVsTime[:,1][0], positionVsTime[:,2][0], positionVsTime[:,3][0]])

                    waypoints = np.array(waypoints)
                    ax.scatter3D(waypoints[:, 0], waypoints[:, 1], waypoints[:, 2], color=color)

        ax.set_box_aspect([ub - lb for lb, ub in (getattr(ax, f'get_{a}lim')() for a in 'xyz')])
        ax.set_xlabel('WCS x (m)')
        ax.set_ylabel('WCS y (m)')
        ax.set_zlabel('WCS z (m)')
        ax.legend(handlelength=4.0)

    def plotPosition2D(self, platformList, plotWaypoints=True, runNumberList=[1]):
        """
        Takes a list of platforms and, optionally, a list of run numbers, and plots each platform from each run in a 2D position plot
        """
        ax = plt.axes()
        for i, n in enumerate(runNumberList):
            for p in platformList:
                platformInfo = self._getPlatformInfo(p, n)
                positionVsTime = np.array(platformInfo['locationLLA'])
                if (len(runNumberList) == 1):
                    labelStr = p
                else:
                    labelStr = f"{p}:{n}"

                color = afsimColors[platformInfo['color']]
                ax.plot(positionVsTime[:, 2], positionVsTime[:, 1], label=labelStr, color=color, linestyle=lineStyles[i%len(lineStyles)])
                if plotWaypoints and 'route' in platformInfo:
                    waypoints = []
                    for waypoint in platformInfo['route']:
                        lat = waypoint['locationX']
                        long = waypoint['locationY']
                        waypoints.append([long, lat])

                    waypoints = np.array(waypoints)
                    ax.scatter(waypoints[:, 0], waypoints[:, 1], color=color)

        ax.set_xlabel('Longitude')
        ax.set_ylabel('Latitude')
        ax.legend(handlelength=4.0)

    def plotPlatformCounts(self, runNumberList=[1], colorList=None, stackplot=False, pyplot_axis=None):
        """
        Creates a (optionally) stacked plot of the number of platforms of each side over the simulation's time
        """
        if not pyplot_axis:
            ax = plt.axes()
        else:
            ax = pyplot_axis

        for i, n in enumerate(runNumberList):
            proc = self.fileProcessors[n]

            if colorList:
                counts = dict([(key, np.array(proc.platformCounts[key])) for key in colorList if sum(np.array(proc.platformCounts[key])[:,1]) > 0])
            else:
                counts = dict([(key, np.array(proc.platformCounts[key])) for key in afsimColors if sum(np.array(proc.platformCounts[key])[:,1]) > 0])

            if stackplot:
                keys = list(counts.keys())
                for j, key in enumerate(keys):
                    for key2 in keys[:j]:
                        sumvals = []
                        for val1, val2 in zip(counts[key], counts[key2]):
                            sumvals.append([val1[0], val1[1] + val2[1]])
                        counts[key2] = np.array(sumvals)

            for key in counts:
                ax.plot(counts[key][:,0], counts[key][:,1], color=afsimColors[key], linestyle=lineStyles[i%len(lineStyles)])

            ax.plot(np.empty((0,1)), np.empty((0,1)), color=afsimColors['black'], label=f"Run {n}", linestyle=lineStyles[i%len(lineStyles)])
            ax.legend(handlelength=4.0)

    def plotFuelLevels(self, platformList, runNumberList=[1], pyplot_axis=None):
        """
        Plots the fuel levels of each platform in [platformList]
        """
        if not pyplot_axis:
            ax = plt.axes()
        else:
            ax = pyplot_axis

        for i, n in enumerate(runNumberList):
            for j, p in enumerate(platformList):
                platformInfo = self._getPlatformInfo(p, n)
                fuelVsTime = np.array(platformInfo['fuel'])
                if (len(runNumberList) == 1):
                    labelStr = p
                else:
                    labelStr = f"{p}:{n}"

                ax.plot(fuelVsTime[:, 0], fuelVsTime[:, 1], label=labelStr, linestyle=lineStyles[i%len(lineStyles)])

        ax.set_xlabel('Time (s)')
        ax.set_ylabel('Fuel')
        ax.legend(handlelength=4.0)

    def plotAltitudeVsTime(self, platformList, runNumberList=[1], pyplot_axis=None):
        """
        Plots the altitudes of each platform side-by-side in a 3D plot
        """
        if not pyplot_axis:
            ax = plt.axes()
        else:
            ax = pyplot_axis

        for i, n in enumerate(runNumberList):
            for p in platformList:
                platformInfo = self._getPlatformInfo(p, n)
                positionVsTime = np.array(platformInfo['locationLLA'])
                if (len(runNumberList) == 1):
                    labelStr = p
                else:
                    labelStr = f"{p}:{n}"

                ax.plot(positionVsTime[:, 0], positionVsTime[:, 3], label=labelStr, linestyle=lineStyles[i%len(lineStyles)])

        ax.set_xlabel('Time (s)')
        ax.set_ylabel('Altitude (m)')
        ax.legend(handlelength=4.0)

    def plotAmmoQuantityVsTime(self, platformComponentList, runNumberList = [1], pyplot_axis=None):
        """
        Plots the ammo quantity of each platform/component pair
        """
        if not pyplot_axis:
            ax = plt.axes()
        else:
            ax = pyplot_axis

        for i, n in enumerate(runNumberList):
            proc = self.fileProcessors[n]
            for (p, c) in platformComponentList:
                if (len(runNumberList) == 1):
                    labelStr = f"{p}:{c}"
                else:
                    labelStr = f"{p}:{c}:{n}"

                quantity = np.array(proc.weaponInfo[p][c]['quantity'])
                ax.plot(quantity[:, 0], quantity[:, 1], label=labelStr, linestyle=lineStyles[i%len(lineStyles)])

        ax.set_xlabel('Time (s)')
        ax.set_ylabel('Ammo Quantity')
        ax.legend(handlelength=4.0)

    def plotProbabilityOfDetection(self, sensorPlatform, sensorName, targetPlatform, runNumberList=[1], pyplot_axis=None):
        """
        Plots probability of detection from a sensor to a target over time, if applicable
        """
        if not pyplot_axis:
            ax = plt.axes()
        else:
            ax = pyplot_axis

        for i, n in enumerate(runNumberList):
            sensorInfo = self._getSensorDetectInfo(sensorPlatform, sensorName, targetPlatform, n)
            if sensorInfo['pd']:
                pd = np.array(sensorInfo['pd'])
                if (len(runNumberList) == 1):
                    labelStr = f"{sensorPlatform}:{sensorName} -> {targetPlatform}"
                else:
                    labelStr = f"{sensorPlatform}:{sensorName} -> {targetPlatform}:{n}"

                ax.plot(pd[:, 0], pd[:, 1], label=labelStr, linestyle=lineStyles[i%len(lineStyles)])

            else:
                print(f"No probability of detection data available for\n\trunNumber: {n}\n\thostPlatform: {sensorPlatform}\n\tsensorName: {sensorName}\n\ttargetPlatform: {targetPlatform}")

        ax.set_xlabel('Time (s)')
        ax.set_ylabel('Detection probability')
        ax.legend(handlelength=4.0)

    def plotPixelCount(self, sensorPlatform, sensorName, targetPlatform, runNumberList=[1], pyplot_axis=None):
        """
        Plots pixel count from a sensor to a target over time, if applicable
        """
        if not pyplot_axis:
            ax = plt.axes()
        else:
            ax = pyplot_axis

        for i, n in enumerate(runNumberList):
            sensorInfo = self._getSensorDetectInfo(sensorPlatform, sensorName, targetPlatform, n)
            if sensorInfo['pixelCount']:
                pixelCount = np.array(sensorInfo['pixelCount'])
                if (len(runNumberList) == 1):
                    labelStr = f"{sensorPlatform}:{sensorName} -> {targetPlatform}"
                else:
                    labelStr = f"{sensorPlatform}:{sensorName} -> {targetPlatform}:{n}"

                ax.plot(pixelCount[:, 0], pixelCount[:, 1], linestyle=lineStyles[i%len(lineStyles)], label=labelStr)

            else:
                print(f"No pixel count data available for\n\trunNumber: {n}\n\thostPlatform: {sensorPlatform}\n\tsensorName: {sensorName}\n\ttargetPlatform: {targetPlatform}")

        ax.set_xlabel('Time (s)')
        ax.set_ylabel('Pixel count')
        ax.legend(handlelength=4.0)

    def plotSignalToNoise(self, sensorPlatform, sensorName, targetPlatform, runNumberList=[1], pyplot_axis=None):
        """
        Plots signal-to-noise ratio from a sensor to a target over time, if applicable
        """
        if not pyplot_axis:
            ax = plt.axes()
        else:
            ax = pyplot_axis

        for i, n in enumerate(runNumberList):
            sensorInfo = self._getSensorDetectInfo(sensorPlatform, sensorName, targetPlatform, n)
            if sensorInfo['signalToNoise']:
                snr = np.array(sensorInfo['signalToNoise'])
                if (len(runNumberList) == 1):
                    labelStr = f"{sensorPlatform}:{sensorName} -> {targetPlatform}"
                else:
                    labelStr = f"{sensorPlatform}:{sensorName} -> {targetPlatform}:{n}"

                ax.plot(snr[:, 0], snr[:, 1], linestyle=lineStyles[i%len(lineStyles)], label=labelStr)

            else:
                print(f"No signal-to-noise data available for\n\trunNumber: {n}\n\thostPlatform: {sensorPlatform}\n\tsensorName: {sensorName}\n\ttargetPlatform: {targetPlatform}")

        ax.set_xlabel('Time (s)')
        ax.set_ylabel('Signal-to-Noise ratio')
        ax.legend(handlelength=4.0)

    def plotComparisonDistance(self, baseline, comparison, pyplot_axis=None):
        """
        Plots the distance between [baseline] and [comparison]

        Parameters:
            baseline : 2-tuple (string, int)
                2-tuple of the form (name, runNumber), e.g. ('mr_mover_red', 1)

            comparison : 2-tuple (string, int)
                2-tuple of the form (name, runNumber), e.g. ('mr_mover_red', 2)

            pyplot_axis: matplotlib.pyplot.axes
                If included, will plot on an existing graph. Use this to plot multiple comparisons on the same graph

        """
        platformInfoBase = self._getPlatformInfo(*baseline)
        platformInfoComp = self._getPlatformInfo(*comparison)
        arraydelta = makeDeltaArray(platformInfoBase['locationWCS'], platformInfoComp['locationWCS'])
        arraydistance = np.array(list(map(lambda a: [a[0], math.sqrt(a[1]*a[1] + a[2]*a[2] + a[3]*a[3])], arraydelta)))

        if not pyplot_axis:
            ax = plt.axes()
        else:
            ax = pyplot_axis

        ax.plot(arraydistance[:, 0], arraydistance[:, 1], label=f"Distance({baseline[0]}:{baseline[1]}, {comparison[0]}:{comparison[1]})")
        ax.set_xlabel('Time (s)')
        ax.set_ylabel('Distance (m)')
        ax.legend()

    def plotComparisonFuelLevels(self, baseline, comparison, pyplot_axis=None):
        """
        Plots the fuel levels between [baseline] and [comparison]

        Parameters:
            baseline : 2-tuple (string, int)
                2-tuple of the form (name, runNumber), e.g. ('mr_mover_red', 1)

            comparison : 2-tuple (string, int)
                2-tuple of the form (name, runNumber), e.g. ('mr_mover_red', 2)

            pyplot_axis: matplotlib.pyplot.axes
                If included, will plot on an existing graph. Use this to plot multiple comparisons on the same graph

        """
        platformInfoBase = self._getPlatformInfo(*baseline)
        platformInfoComp = self._getPlatformInfo(*comparison)
        arraydelta = np.array(makeDeltaArray(platformInfoBase['fuel'], platformInfoComp['fuel']))

        if not pyplot_axis:
            ax = plt.axes()
        else:
            ax = pyplot_axis

        ax.plot(arraydelta[:, 0], arraydelta[:, 1], label=f"Fuel({baseline[0]}:{baseline[1]}) - Fuel({comparison[0]}:{comparison[1]})")
        ax.set_xlabel('Time (s)')
        ax.set_ylabel('Fuel')
        ax.legend()

    def plotComparisonAltitude(self, baseline, comparison, pyplot_axis=None):
        """
        Plots relative altitudes between [baseline] and [comparison]

        Parameters:
            baseline : 2-tuple (string, int)
                2-tuple of the form (name, runNumber), e.g. ('mr_mover_red', 1)

            comparison : 2-tuple (string, int)
                2-tuple of the form (name, runNumber), e.g. ('mr_mover_red', 2)

            pyplot_axis: matplotlib.pyplot.axes
                If included, will plot on an existing graph. Use this to plot multiple comparisons on the same graph

        """
        platformInfoBase = self._getPlatformInfo(*baseline)
        platformInfoComp = self._getPlatformInfo(*comparison)
        arraydelta = makeDeltaArray(platformInfoBase['locationLLA'], platformInfoComp['locationLLA'])
        arraydistance = np.array(list(map(lambda a: [a[0], a[3]], arraydelta)))

        if not pyplot_axis:
            ax = plt.axes()
        else:
            ax = pyplot_axis

        ax.plot(arraydistance[:, 0], arraydistance[:, 1], label=f"Altitude({comparison[0]}:{comparison[1]}) - Altitude({baseline[0]}:{baseline[1]})")
        ax.set_xlabel('Time (s)')
        ax.set_ylabel('Relative altitude (m)')
        ax.legend()

    def plotComparisonProbabilityOfDetection(self, baseline, comparison, pyplot_axis=None):
        """
        Plots the relative probability of detection between [baseline] and [comparison]

        Parameters:
            baseline : 4-tuple (string, string, string, int)
                4-tuple of the form (sensorPlatform, sensorName, targetPlatform, runNumber), e.g. ('irst_sensor', 'sensor_irst', 'fighter_multi', 1)

            comparison : 4-tuple (string, string, string, int)
                4-tuple of the form (sensorPlatform, sensorName, targetPlatform, runNumber), e.g. ('irst_sensor', 'sensor_irst', 'fighter_multi', 2)

            pyplot_axis: matplotlib.pyplot.axes
                If included, will plot on an existing graph. Use this to plot multiple comparisons on the same graph

        """
        sensorInfoBase = self._getSensorDetectInfo(*baseline)
        sensorInfoComp = self._getSensorDetectInfo(*comparison)
        arraydelta = np.array(makeDeltaArray(sensorInfoBase['pd'], sensorInfoComp['pd']))

        if not pyplot_axis:
            ax = plt.axes()
        else:
            ax = pyplot_axis

        ax.plot(arraydelta[:, 0], arraydelta[:, 1], label=f"  pd({comparison[0]}:{comparison[1]} -> {comparison[2]}:{comparison[3]})\n- pd({baseline[0]}:{baseline[1]} -> {baseline[2]}:{baseline[3]})")
        ax.set_xlabel('Time (s)')
        ax.set_ylabel('Detection probability')
        ax.legend()

    def plotComparisonSignalToNoise(self, baseline, comparison, pyplot_axis=None):
        """
        Plots the relative signal-to-noise ratio between [baseline] and [comparison]

        Parameters:
            baseline : 4-tuple (string, string, string, int)
                4-tuple of the form (sensorPlatform, sensorName, targetPlatform, runNumber), e.g. ('irst_sensor', 'sensor_irst', 'fighter_multi', 1)

            comparison : 4-tuple (string, string, string, int)
                4-tuple of the form (sensorPlatform, sensorName, targetPlatform, runNumber), e.g. ('irst_sensor', 'sensor_irst', 'fighter_multi', 2)

            pyplot_axis: matplotlib.pyplot.axes
                If included, will plot on an existing graph. Use this to plot multiple comparisons on the same graph

        """
        sensorInfoBase = self._getSensorDetectInfo(*baseline)
        sensorInfoComp = self._getSensorDetectInfo(*comparison)
        arraydelta = np.array(makeDeltaArray(sensorInfoBase['signalToNoise'], sensorInfoComp['signalToNoise']))

        if not pyplot_axis:
            ax = plt.axes()
        else:
            ax = pyplot_axis

        ax.plot(arraydelta[:, 0], arraydelta[:, 1], label=f"  snr({comparison[0]}:{comparison[1]} -> {comparison[2]}:{comparison[3]})\n- snr({baseline[0]}:{baseline[1]} -> {baseline[2]}:{baseline[3]})")
        ax.set_xlabel('Time (s)')
        ax.set_ylabel('Signal-to-noise ratio')
        ax.legend()

    def plotComparisonPixelCount(self, baseline, comparison, pyplot_axis=None):
        """
        Plots the relative pixel count between [baseline] and [comparison]

        Parameters:
            baseline : 4-tuple (string, string, string, int)
                4-tuple of the form (sensorPlatform, sensorName, targetPlatform, runNumber), e.g. ('irst_sensor', 'sensor_irst', 'fighter_multi', 1)

            comparison : 4-tuple (string, string, string, int)
                4-tuple of the form (sensorPlatform, sensorName, targetPlatform, runNumber), e.g. ('irst_sensor', 'sensor_irst', 'fighter_multi', 2)

            pyplot_axis: matplotlib.pyplot.axes
                If included, will plot on an existing graph. Use this to plot multiple comparisons on the same graph

        """
        sensorInfoBase = self._getSensorDetectInfo(*baseline)
        sensorInfoComp = self._getSensorDetectInfo(*comparison)
        arraydelta = np.array(makeDeltaArray(sensorInfoBase['pd'], sensorInfoComp['pd']))

        if not pyplot_axis:
            ax = plt.axes()
        else:
            ax = pyplot_axis

        ax.plot(arraydelta[:, 0], arraydelta[:, 1], label=f"  pixelCount({comparison[0]}:{comparison[1]} -> {comparison[2]}:{comparison[3]})\n- pixelCount({baseline[0]}:{baseline[1]} -> {baseline[2]}:{baseline[3]})")
        ax.set_xlabel('Time (s)')
        ax.set_ylabel('Pixel Count')
        ax.legend()
