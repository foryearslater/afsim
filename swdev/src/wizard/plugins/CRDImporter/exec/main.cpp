// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2019 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include <iostream>
#include <string>
#include <vector>

#include "CrdFileImporter.hpp"
#include "UtPath.hpp"

static void ShowUsage(const std::string& name)
{
   // clang-format off
      std::cerr << "Usage: " << name.c_str() << " <option(s)> SOURCES\n"
         << "Options:\n"
         << "\t-h,--help\t\t\tShow this help message.\n"
         << "\t-d,--debug\t\t\tAdd source debugging information in Mission route file.\n"
         << "\t-t,--tolerance  <double-value> \tSpecify tolerance for equality of geographic lat/lon data.\n"
         << "\t-r,--recurseTransitions <level> Level is true or false. false generates routes with path level transitions. true\n"
         << "\t\t\t\t\trecurses all subordinate transition_list elements and includes them in the routes. Default value is true.\n"
         << "\t-f,--format\t\t\tSpecify format for lat/lon coordinates in output Mission Route files. Options are DECIMAL and DMS.\n"
         //<< "\t-i,--inputfile\t\t<filename> Specify an input CRD file or folder to be processed into AFSIM Route Files."
         //<< "\t\t Multiple entries can be specified by using a comma separated list (with no spaces).\n"
         << "\t-o,--outputfilebase <filename> \tSpecify the base name of the AFSIM Mission Route output file(s).\n"
         << "\t-odir,--odir <directory-name> \tSpecify the name of an output folder where the output files will be placed.\n"
//         << "\t-idir,--idir <directory-name> \tSpecify the name of an input folder where the input files will be read. (NOT IMPLEMENTED)\n"
         << std::endl;
   // clang-format on
}

int main(int argc, char* argv[])
{
   UtPath path(".");
   std::cout << "Path of current directory is " << path.GetRealPath().GetSystemPath().c_str() << std::endl;

   double geoMeasurementTolerance = 0.000001;
   bool   debug                   = false;
   bool   recurseTransitions      = true;

   CrdImporter::CrdFileImporter::PointFormatType pointFormat        = CrdImporter::CrdFileImporter::cDMS_FORMAT;
   std::string                                   inputFolder        = ".";
   std::string                                   outputFolder       = ".";
   std::string                                   outputFilenameBase = "";

   if (argc < 2)
   {
      ShowUsage(argv[0]);
      return 1;
   }

   std::vector<std::string> sources;
   std::string              destination;
   for (int i = 1; i < argc; ++i)
   {
      std::string arg = argv[i];
      if ((arg == "-h") || (arg == "--help"))
      {
         ShowUsage(argv[0]);
         return 0;
      }
      else if ((arg == "-d") || (arg == "--debug"))
      {
         debug = true;
      }
      else if ((arg == "-t") || (arg == "--tolerance"))
      {
         if (i + 1 < argc)
         { // Make sure we aren't at the end of argv!
            bool success = false;
            try
            {
               size_t      nextCharIndex   = -1;
               std::string toleranceString = argv[++i];
               ;
               geoMeasurementTolerance = std::stod(toleranceString, &nextCharIndex);

               success = (nextCharIndex == toleranceString.length()) && (geoMeasurementTolerance > 0.0) &&
                         (geoMeasurementTolerance < 1.0);
            }
            catch (std::exception&)
            {
            }

            if (!success)
            {
               std::cerr << "--tolerance option requires a floating point value between 0 and 1." << std::endl;
               return 1;
            }
         }
         else
         {
            std::cerr << "--tolerance option requires one argument." << std::endl;
            return 1;
         }
      }
      else if ((arg == "-r") || (arg == "--recurseTransitions"))
      {
         if (i + 1 < argc)
         {                                         // Make sure we aren't at the end of argv!
            std::string recurseString = argv[++i]; // Increment 'i' so we don't get the argument as the next argv[i].
            if (recurseString == "true")
            {
               recurseTransitions = true;
            }
            else
            {
               recurseTransitions = false;
            }
         }
         else
         {
            std::cerr << "--recurseTransitions option requires one argument." << std::endl;
            return 1;
         }
      }
      else if ((arg == "-f") || (arg == "--format"))
      {
         if (i + 1 < argc)
         {                                  // Make sure we aren't at the end of argv!
            std::string format = argv[++i]; // Increment 'i' so we don't get the argument as the next argv[i].
            if (format == "DECIMAL")
            {
               pointFormat = CrdImporter::CrdFileImporter::cDECIMAL_FORMAT;
            }
            else
            {
               pointFormat = CrdImporter::CrdFileImporter::cDMS_FORMAT;
            }
         }
         else
         {
            std::cerr << "--format option requires one argument." << std::endl;
            return 1;
         }
      }
      else if ((arg == "-o") || (arg == "--outputfilebase"))
      {
         if (i + 1 < argc)
         {                                  // Make sure we aren't at the end of argv!
            outputFilenameBase = argv[++i]; // Increment 'i' so we don't get the argument as the next argv[i].
         }
         else
         {
            std::cerr << "--outputfilebase option requires one argument." << std::endl;
            return 1;
         }
      }
      else if ((arg == "-odir") || (arg == "--odir"))
      {
         if (i + 1 < argc)
         {                            // Make sure we aren't at the end of argv!
            outputFolder = argv[++i]; // Increment 'i' so we don't get the argument as the next argv[i].
         }
         else
         {
            std::cerr << "--odir option requires one argument." << std::endl;
            return 1;
         }
      }
      else if ((arg == "-idir") || (arg == "--idir"))
      {
         if (i + 1 < argc)
         {                           // Make sure we aren't at the end of argv!
            inputFolder = argv[++i]; // Increment 'i' so we don't get the argument as the next argv[i].
            std::cerr << "--idir option is not implemented." << std::endl;
         }
         else
         {
            std::cerr << "--idir option requires one argument.(NOT IMPLEMENTED)" << std::endl;
            return 1;
         }
      }
      else
      {
         std::string currFile = argv[i];
         UtPath      path(currFile);
         if (path.Exists())
         {
            if (path.IsDirectory())
            {
               // look for all crd files in the folder specified
               std::vector<std::string> files;
               std::vector<std::string> directories;
               path.ListDir(files, directories);
               // not recursing, don't need the directories
               if (!files.empty())
               {
                  std::string suffix     = ".crd";
                  auto        suffixSize = suffix.size();
                  for (std::string currFile : files)
                  {
                     transform(currFile.begin(), currFile.end(), currFile.begin(), ::tolower);
                     auto filenameSize = currFile.size();
                     bool isCrdFile    = filenameSize > suffixSize &&
                                      currFile.compare(filenameSize - suffixSize, suffixSize, suffix) == 0;
                     if (isCrdFile)
                     {
                        sources.emplace_back(path.GetRealPath().GetSystemPath() + "/" + currFile);
                     }
                  }
               }
            }
            else
            {
               sources.emplace_back(argv[i]);
            }
         }
         else
         {
            std::cout << "The input file " << argv[i] << " specified on the command line does not exist." << std::endl;
         }
      }
   }
   // return move(sources, destination);

   CrdImporter::CrdFileImporter crdImporter;
   crdImporter.SetDebug(debug);
   crdImporter.SetOutputFolder(outputFolder);
   crdImporter.SetGeoEqualityTolerance(geoMeasurementTolerance);
   crdImporter.SetPointFormat(pointFormat);
   crdImporter.SetRecurseTransitions(recurseTransitions);
   crdImporter.SetOutputFilenameBase(outputFilenameBase);
   crdImporter.SetSourceCrdInputFiles(sources);
   std::string test = "";
   crdImporter.ReplaceIllegalCharacters(test);
   crdImporter.ParseCrdFiles();
}
