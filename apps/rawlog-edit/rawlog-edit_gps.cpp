/* +------------------------------------------------------------------------+
   |                     Mobile Robot Programming Toolkit (MRPT)            |
   |                          https://www.mrpt.org/                         |
   |                                                                        |
   | Copyright (c) 2005-2021, Individual contributors, see AUTHORS file     |
   | See: https://www.mrpt.org/Authors - All rights reserved.               |
   | Released under BSD License. See: https://www.mrpt.org/License          |
   +------------------------------------------------------------------------+ */

#include <mrpt/obs/CObservationGPS.h>
#include <mrpt/topography.h>

#include "rawlog-edit-declarations.h"

using namespace mrpt;
using namespace mrpt::obs;
using namespace mrpt::poses;
using namespace mrpt::system;
using namespace mrpt::math;
using namespace mrpt::rawlogtools;
using namespace mrpt::topography;
using namespace std;
using namespace mrpt::io;

// STL data must have global scope:
struct TGPSDataPoint
{
	double lon, lat, alt;  // degrees, degrees, meters
	uint8_t fix;  // 1: standalone, 2: DGPS, 4: RTK fix, 5: RTK float, ...
};

struct TDataPerGPS
{
	map<TTimeStamp, TGPSDataPoint> path;
};

// ======================================================================
//		op_export_gps_kml
// ======================================================================
DECLARE_OP_FUNCTION(op_export_gps_kml)
{
	// A class to do this operation:
	class CRawlogProcessor_ExportGPS_KML
		: public CRawlogProcessorOnEachObservation
	{
	   protected:
		string m_inFile;

		map<string, TDataPerGPS> m_gps_paths;  // sensorLabel -> data

	   public:
		CRawlogProcessor_ExportGPS_KML(
			CFileGZInputStream& in_rawlog, TCLAP::CmdLine& cmdline,
			bool _verbose)
			: CRawlogProcessorOnEachObservation(in_rawlog, cmdline, _verbose)
		{
			getArgValue<string>(cmdline, "input", m_inFile);
		}

		// return false on any error.
		bool processOneObservation(CObservation::Ptr& o) override
		{
			if (!IS_CLASS(*o, CObservationGPS)) return true;

			const CObservationGPS* obs =
				dynamic_cast<CObservationGPS*>(o.get());

			if (!obs->has_GGA_datum()) return true;	 // Nothing to do...

			// Insert the new entries:
			TDataPerGPS& D = m_gps_paths[obs->sensorLabel];
			TGPSDataPoint& d = D.path[o->timestamp];
			const auto& gga =
				obs->getMsgByClass<mrpt::obs::gnss::Message_NMEA_GGA>();
			d.lon = gga.fields.longitude_degrees;
			d.lat = gga.fields.latitude_degrees;
			d.alt = gga.fields.altitude_meters;
			d.fix = gga.fields.fix_quality;

			return true;  // All ok
		}

		void generate_KML()
		{
			const bool save_altitude = false;

			const string outfilname =
				mrpt::system::fileNameChangeExtension(m_inFile, "kml");
			VERBOSE_COUT << "Writing KML file: " << outfilname << endl;

			CFileOutputStream f(outfilname);

			// Header:
			f.printf(
				"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
				"<kml xmlns=\"http://www.opengis.net/kml/2.2\">\n"
				"<!-- File automatically generated by rawlog-edit \n"
				"      Part of the MRPT initiative - https://www.mrpt.org/ \n"
				"      Generated on %s from file '%s'  -->\n"
				"  <Document>\n"
				"    <name>Paths</name>\n"
				"    <description>GPS paths from dataset '%s'</description>\n",
				mrpt::system::dateTimeLocalToString(mrpt::system::now())
					.c_str(),
				m_inFile.c_str(), m_inFile.c_str());

			// Define a few predefined colors:
			const int LINEWIDTH = 2;
			const int THICKLINEWIDTH = 5;
			const double MIN_DIST_TO_SPLIT = 15.0;

			static const size_t NCOLORS = 5;
			static const char* COLOR_CODES[] = {
				"a000ffff", "a00000ff", "a0ff0000", "a0707070", "a0000000"};

			for (size_t i = 0; i < NCOLORS; i++)
			{
				f.printf(
					"    <Style id=\"gpscolor%i\">\n"
					"      <LineStyle>\n"
					"        <color>%s</color>\n"
					"        <width>%i</width>\n"
					"      </LineStyle>\n"
					"    </Style>\n",
					int(i), COLOR_CODES[i], LINEWIDTH);
				f.printf(
					"    <Style id=\"gpscolor%i_thick\">\n"
					"      <LineStyle>\n"
					"        <color>%s</color>\n"
					"        <width>%i</width>\n"
					"      </LineStyle>\n"
					"    </Style>\n",
					int(i), COLOR_CODES[i], THICKLINEWIDTH);
			}

			const string LineString_START = format(
				"      <LineString>\n"
				"        %s\n"
				"       <coordinates> \n",
				save_altitude ? "<altitudeMode>absolute</altitudeMode>" : "");
			const string LineString_END =
				"        </coordinates>\n"
				"      </LineString>\n";

			// For each sensor label:
			int color_idx = 0;
			for (auto it = m_gps_paths.begin(); it != m_gps_paths.end();
				 ++it, color_idx++)
			{
				const string& label = it->first;
				const TDataPerGPS& D = it->second;

				bool hasSomeRTK = false;

				f.printf(
					"    <Placemark>\n"
					"      <name>%s all points</name>\n"
					"      <description>%s: All received points (for all "
					"quality levels)</description>\n"
					"      <styleUrl>#gpscolor%i</styleUrl>\n",
					label.c_str(), label.c_str(),
					int(color_idx % NCOLORS)  // Color
				);
				f.printf("%s", LineString_START.c_str());

				for (const auto& itP : D.path)
				{
					const TGPSDataPoint& d = itP.second;
					// Format is: lon,lat[,alt]
					if (save_altitude)
						f.printf(" %.15f,%.15f,%.3f\n", d.lon, d.lat, d.alt);
					else
						f.printf(" %.15f,%.15f\n", d.lon, d.lat);

					if (!hasSomeRTK && d.fix == 4) hasSomeRTK = true;
				}

				// end part:
				f.printf("%s", LineString_END.c_str());

				f.printf("    </Placemark>\n");

				// Do we have RTK points?
				if (hasSomeRTK)
				{
					f.printf(
						"    <Placemark>\n"
						"      <name>%s RTK only</name>\n"
						"      <description>%s: RTK fixed points "
						"only</description>\n"
						"      <styleUrl>#gpscolor%i_thick</styleUrl>\n",
						label.c_str(), label.c_str(),
						int(color_idx % NCOLORS)  // Color
					);

					f.printf(" <MultiGeometry>\n");
					f.printf("%s", LineString_START.c_str());

					TGPSDataPoint last_valid;
					last_valid.lat = 0;
					last_valid.lon = 0;

					for (const auto& itP : D.path)
					{
						const TGPSDataPoint& d = itP.second;
						if (d.fix != 4) continue;  // Skip this one..

						// There was a valid point?
						if (last_valid.lat != 0 && last_valid.lon != 0)
						{
							// Compute distance between points, in meters:
							//  (very rough, but fast spherical approximation):
							const double dist = 6.371e6 *
								DEG2RAD(::hypot(
									last_valid.lon - d.lon,
									last_valid.lat - d.lat));

							// If the distance is above a threshold, finish the
							// line and start another one:
							if (dist > MIN_DIST_TO_SPLIT)
							{
								f.printf("%s", LineString_END.c_str());
								f.printf("%s", LineString_START.c_str());
							}

							// Format is: lon,lat[,alt]
							if (save_altitude)
								f.printf(
									" %.15f,%.15f,%.3f\n", d.lon, d.lat, d.alt);
							else
								f.printf(" %.15f,%.15f\n", d.lon, d.lat);
						}

						// Save last point:
						last_valid = d;
					}

					// end part:
					f.printf("%s", LineString_END.c_str());
					f.printf(" </MultiGeometry>\n");

					f.printf("    </Placemark>\n");
				}

			}  // end for each sensor label

			f.printf(
				"  </Document>\n"
				"</kml>\n");
		}  // end generate_KML

	};	// end CRawlogProcessor_ExportGPS_KML

	// Process
	// ---------------------------------
	CRawlogProcessor_ExportGPS_KML proc(in_rawlog, cmdline, verbose);
	proc.doProcessRawlog();

	// Now that the entire rawlog is parsed, do the actual output:
	proc.generate_KML();

	// Dump statistics:
	// ---------------------------------
	VERBOSE_COUT << "Time to process file (sec)        : " << proc.m_timToParse
				 << "\n";
}

// ======================================================================
//		op_export_gps_txt
// ======================================================================
DECLARE_OP_FUNCTION(op_export_gps_txt)
{
	// A class to do this operation:
	class CRawlogProcessor_ExportGPS_TXT
		: public CRawlogProcessorOnEachObservation
	{
	   protected:
		string m_inFile;

		// All gps data:
		map<TTimeStamp, map<string, CPoint3D>> lstXYZallGPS;
		set<string> lstAllGPSlabels;

		map<TTimeStamp, map<string, CPoint3D>> lstXYZallGPS_RTK;
		set<string> lstAllGPSlabels_RTK;

		map<string, FILE*> lstFiles;
		TGeodeticCoords refCoords;
		CPose3D local_ENU;
		string m_filPrefix;

		void doSaveJointFile(
			map<TTimeStamp, map<string, CPoint3D>>& lstxyz,
			set<string>& lstlabels, const char* gpsKindLabel)
		{
			// Remove those entries with not all the GPSs:
			for (auto a = lstxyz.begin(); a != lstxyz.end();)
			{
				if (a->second.size() != lstlabels.size())
				{
					auto b = a;
					b++;
					lstxyz.erase(a);
					a = b;
				}
				else
					++a;
			}

			VERBOSE_COUT << "Number of timestamps in ALL the " << gpsKindLabel
						 << " GPSs     : " << lstxyz.size() << endl;

			CMatrixDouble MAT(lstxyz.size(), 1 + 3 * lstlabels.size());
			int nLabels = 0;
			for (auto a = lstxyz.begin(); a != lstxyz.end(); ++a, nLabels++)
			{
				MAT(nLabels, 0) = timestampTotime_t(a->first);
				auto& m = a->second;
				int k = 0;
				for (auto it = lstlabels.begin(); it != lstlabels.end();
					 ++it, k++)
				{
					MAT(nLabels, 1 + 3 * k + 0) = m[*it].x();
					MAT(nLabels, 1 + 3 * k + 1) = m[*it].y();
					MAT(nLabels, 1 + 3 * k + 2) = m[*it].z();
				}
			}

			// The name of the file:
			string joint_name;
			for (const auto& lstlabel : lstlabels)
			{
				joint_name += lstlabel;
			}

			const string jointFilName = format(
				"%s_JOINT%s_%s.txt", m_filPrefix.c_str(), gpsKindLabel,
				joint_name.c_str());

			VERBOSE_COUT << "Writing joint GPS file: " << jointFilName << endl;

			MAT.saveToTextFile(
				jointFilName, MATRIX_FORMAT_ENG, false,
				"% For N GPS sensors, each line has 1+3*N entries: \n"
				"% timestamp (UNIX time_t with sec fractions) + N*[ ENU_X "
				"ENU_Y ENU_Z ] \n"
				"% "
				"--------------------------------------------------------------"
				"----- \n");

			CMatrixDouble MAT_REF(1, 3);
			MAT_REF(0, 0) = refCoords.lon;
			MAT_REF(0, 1) = refCoords.lat;
			MAT_REF(0, 2) = refCoords.height;
			MAT_REF.saveToTextFile(
				format(
					"%s_JOINTREF%s_%s.txt", m_filPrefix.c_str(), gpsKindLabel,
					joint_name.c_str()),
				MATRIX_FORMAT_FIXED, false,
				"% Reference geodetic coordinate for ENU's origin of "
				"coordinates: \n"
				"% LON(DEG)    LAT(DEG)    HEIGHT(m) \n"
				"% "
				"--------------------------------------------------------------"
				"----- \n");
		}

	   public:
		size_t m_GPS_entriesSaved;

		CRawlogProcessor_ExportGPS_TXT(
			CFileGZInputStream& in_rawlog, TCLAP::CmdLine& cmdline,
			bool _verbose)
			: CRawlogProcessorOnEachObservation(in_rawlog, cmdline, _verbose),
			  m_GPS_entriesSaved(0)
		{
			getArgValue<string>(cmdline, "input", m_inFile);
			m_filPrefix =
				extractFileDirectory(m_inFile) + extractFileName(m_inFile);
		}

		// return false on any error.
		bool processOneObservation(CObservation::Ptr& o) override
		{
			if (!IS_CLASS(*o, CObservationGPS)) return true;

			const CObservationGPS* obs =
				dynamic_cast<CObservationGPS*>(o.get());

			auto it = lstFiles.find(obs->sensorLabel);

			FILE* f_this;

			if (it == lstFiles.end())  // A new file for this sensorlabel??
			{
				const std::string fileName = m_filPrefix + string("_") +
					fileNameStripInvalidChars(obs->sensorLabel) +
					string(".txt");

				VERBOSE_COUT << "Writing GPS TXT file: " << fileName << endl;

				f_this = lstFiles[obs->sensorLabel] =
					os::fopen(fileName.c_str(), "wt");
				if (!f_this)
					THROW_EXCEPTION_FMT(
						"Cannot open output file for write: %s",
						fileName.c_str());

				// The first line is a description of the columns:
				::fprintf(
					f_this,
					"%% "
					"%14s "	 // Time
					"%23s %23s %23s "  // lat lon alt
					"%4s %4s %11s %11s "  // fix #sats speed dir
					"%23s %23s %23s "  // X Y Z local
					"%6s "	// rawlog index
					"%21s %21s %21s "  // X Y Z geocentric
					"%21s %21s %21s "  // X Y Z Cartessian (GPS)
					"%21s %21s %21s "  // VX VY VZ Cartessian (GPS)
					"%21s %21s %21s "  // VX VY VZ Cartessian (Local)
					"%14s "	 // SAT Time
					"\n",
					"Time", "Lat", "Lon", "Alt", "fix", "#sats", "speed", "dir",
					"Local X", "Local Y", "Local Z", "rawlog ID", "Geocen X",
					"Geocen Y", "Geocen Z", "GPS X", "GPS Y", "GPS Z", "GPS VX",
					"GPS VY", "GPS VZ", "Local VX", "Local VY", "Local VZ",
					"SAT Time");
			}
			else
				f_this = it->second;

			if (obs->has_GGA_datum())
			{
				const auto& gga =
					obs->getMsgByClass<mrpt::obs::gnss::Message_NMEA_GGA>();
				TPoint3D p;	 // Transformed coordinates

				// The first gps datum?
				if (refCoords.isClear())
				{
					refCoords = gga.getAsStruct<TGeodeticCoords>();

					// Local coordinates reference:
					TPose3D _local_ENU;
					mrpt::topography::ENU_axes_from_WGS84(
						refCoords, _local_ENU, true);
					local_ENU = mrpt::poses::CPose3D(_local_ENU);
				}

				// Local XYZ coordinates transform:
				mrpt::topography::geodeticToENU_WGS84(
					gga.getAsStruct<TGeodeticCoords>(), p, refCoords);

				// Geocentric XYZ:
				TPoint3D geo;
				mrpt::topography::geodeticToGeocentric_WGS84(
					gga.getAsStruct<TGeodeticCoords>(), geo);

				// Save file:
				double tim = mrpt::system::timestampTotime_t(obs->timestamp);

				// If available, Cartessian X Y Z, VX VY VZ, as supplied by the
				// GPS itself:
				TPoint3D cart_pos(0, 0, 0), cart_vel(0, 0, 0);
				TPoint3D cart_vel_local(0, 0, 0);
				if (obs->messages.count(mrpt::obs::gnss::TOPCON_PZS) != 0)
				{
					const auto& pzs = obs->getMsgByClass<
						mrpt::obs::gnss::Message_TOPCON_PZS>();
					if (pzs.hasCartesianPosVel)
					{
						cart_pos.x = pzs.cartesian_x;
						cart_pos.y = pzs.cartesian_y;
						cart_pos.z = pzs.cartesian_z;
						cart_vel.x = pzs.cartesian_vx;
						cart_vel.y = pzs.cartesian_vy;
						cart_vel.z = pzs.cartesian_vz;

						cart_vel_local =
							(CPoint3D(cart_vel) - local_ENU).asTPoint();
					}
				}

				::fprintf(
					f_this,
					"%14.4f "  // Time
					"%23.16f %23.16f %23.6f "  // lat lon alt
					"%4u %4u %11.6f %11.6f "  // fix #sats speed dir
					"%23.16f %23.16f %23.16f "	// X Y Z local
					"%6i "	// rawlog index
					"%21.16f %21.16f %21.16f "	// X Y Z geocentric
					"%21.16f %21.16f %21.16f "	// X Y Z Cartessian (GPS)
					"%21.16f %21.16f %21.16f "	// VX VY VZ Cartessian (GPS)
					"%21.16f %21.16f %21.16f "	// VX VY VZ Cartessian (Local)
					"%14.4f "  // SAT Time
					"\n",
					tim, DEG2RAD(gga.fields.latitude_degrees),
					DEG2RAD(gga.fields.longitude_degrees),
					gga.fields.altitude_meters, gga.fields.fix_quality,
					gga.fields.satellitesUsed,
					obs->has_RMC_datum()
						? DEG2RAD(obs->getMsgByClass<
										 mrpt::obs::gnss::Message_NMEA_RMC>()
									  .fields.speed_knots)
						: 0.0,
					obs->has_RMC_datum()
						? DEG2RAD(obs->getMsgByClass<
										 mrpt::obs::gnss::Message_NMEA_RMC>()
									  .fields.direction_degrees)
						: 0.0,
					p.x, p.y, p.z,
					(int)m_rawlogEntry,	 // rawlog index
					geo.x, geo.y, geo.z, cart_pos.x, cart_pos.y, cart_pos.z,
					cart_vel.x, cart_vel.y, cart_vel.z, cart_vel_local.x,
					cart_vel_local.y, cart_vel_local.z,
					mrpt::system::timestampTotime_t(
						gga.fields.UTCTime.getAsTimestamp(obs->timestamp)));

				m_GPS_entriesSaved++;

				{
					lstAllGPSlabels.insert(obs->sensorLabel);
					lstXYZallGPS[obs->timestamp][obs->sensorLabel] =
						CPoint3D(p);
				}

				if (gga.fields.fix_quality == 4)
				{
					lstAllGPSlabels_RTK.insert(obs->sensorLabel);
					lstXYZallGPS_RTK[obs->timestamp][obs->sensorLabel] =
						CPoint3D(p);
				}
			}
			return true;  // All ok
		}

		// Destructor: close files and generate summary files:
		~CRawlogProcessor_ExportGPS_TXT()
		{
			for (auto it = lstFiles.begin(); it != lstFiles.end(); ++it)
			{
				os::fclose(it->second);
			}
			lstFiles.clear();

			// Save the joint file:
			// -------------------------
			VERBOSE_COUT << "Number of different GPS sensorLabels     : "
						 << lstAllGPSlabels.size() << endl;
			VERBOSE_COUT << "Number of different RTK GPS sensorLabels : "
						 << lstAllGPSlabels_RTK.size() << endl;

			if (!lstAllGPSlabels.empty())
				doSaveJointFile(lstXYZallGPS, lstAllGPSlabels, "");

			if (!lstAllGPSlabels_RTK.empty())
				doSaveJointFile(lstXYZallGPS_RTK, lstAllGPSlabels_RTK, "RTK");

		}  // end of destructor
	};

	// Process
	// ---------------------------------
	CRawlogProcessor_ExportGPS_TXT proc(in_rawlog, cmdline, verbose);
	proc.doProcessRawlog();

	// Dump statistics:
	// ---------------------------------
	VERBOSE_COUT << "Time to process file (sec)        : " << proc.m_timToParse
				 << "\n";
	VERBOSE_COUT << "Number of records saved           : "
				 << proc.m_GPS_entriesSaved << "\n";
}

// ======================================================================
//		op_export_gps_all
// ======================================================================
DECLARE_OP_FUNCTION(op_export_gps_all)
{
	// A class to do this operation:
	class CRawlogProcessor_ExportGPS_ALL
		: public CRawlogProcessorOnEachObservation
	{
	   protected:
		string m_inFile;
		map<string, FILE*> lstFiles;
		string m_filPrefix;

	   public:
		size_t m_GPS_entriesSaved;

		CRawlogProcessor_ExportGPS_ALL(
			CFileGZInputStream& in_rawlog, TCLAP::CmdLine& cmdline,
			bool _verbose)
			: CRawlogProcessorOnEachObservation(in_rawlog, cmdline, _verbose),
			  m_GPS_entriesSaved(0)
		{
			getArgValue<string>(cmdline, "input", m_inFile);
			m_filPrefix =
				extractFileDirectory(m_inFile) + extractFileName(m_inFile);
		}

		// return false on any error.
		bool processOneObservation(CObservation::Ptr& o) override
		{
			if (!IS_CLASS(*o, CObservationGPS)) return true;

			const CObservationGPS* obs =
				dynamic_cast<CObservationGPS*>(o.get());

			for (auto it = obs->messages.begin(); it != obs->messages.end();
				 ++it)
			{
				const gnss::gnss_message* msg_ptr = it->second.get();
				if (!msg_ptr) continue;

				const string sMsg = msg_ptr->getMessageTypeAsString();
				if (sMsg.empty()) continue;

				const string sLabelMsg =
					string(obs->sensorLabel) + string("_MSG_") + sMsg;
				auto itF = lstFiles.find(sLabelMsg);

				FILE* f_this;

				if (itF == lstFiles.end())	// A new file for this sensorlabel??
				{
					const std::string fileName = m_filPrefix + string("_") +
						fileNameStripInvalidChars(sLabelMsg) + string(".txt");

					VERBOSE_COUT << "Writing GPS TXT file: " << fileName
								 << endl;

					f_this = lstFiles[sLabelMsg] =
						os::fopen(fileName.c_str(), "wt");
					if (!f_this)
						THROW_EXCEPTION_FMT(
							"Cannot open output file for write: %s",
							fileName.c_str());

					// The first line is a description of the columns:
					std::stringstream buf;
					msg_ptr->getAllFieldDescriptions(buf);
					::fprintf(
						f_this,
						"%% %16s %16s %s\n%% ------------------------\n",
						"GPS_UNIX_time", "PC_UNIX_time", buf.str().c_str());
				}
				else
					f_this = itF->second;

				std::stringstream buf;
				msg_ptr->getAllFieldValues(buf);
				::fprintf(
					f_this, "%16.06f %16.06f %s\n",
					mrpt::system::timestampTotime_t(obs->timestamp),
					mrpt::system::timestampTotime_t(
						obs->originalReceivedTimestamp),
					buf.str().c_str());
				m_GPS_entriesSaved++;

			}  // for each msg
			return true;  // All ok
		}

		// Destructor: close files and generate summary files:
		~CRawlogProcessor_ExportGPS_ALL()
		{
			VERBOSE_COUT << "Number of different files saved   : "
						 << lstFiles.size() << endl;

			for (auto it = lstFiles.begin(); it != lstFiles.end(); ++it)
			{
				os::fclose(it->second);
			}
			lstFiles.clear();

		}  // end of destructor
	};

	// Process
	// ---------------------------------
	CRawlogProcessor_ExportGPS_ALL proc(in_rawlog, cmdline, verbose);
	proc.doProcessRawlog();

	// Dump statistics:
	// ---------------------------------
	VERBOSE_COUT << "Time to process file (sec)        : " << proc.m_timToParse
				 << "\n";
	VERBOSE_COUT << "Number of records saved           : "
				 << proc.m_GPS_entriesSaved << "\n";
}
