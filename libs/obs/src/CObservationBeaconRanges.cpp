/* +------------------------------------------------------------------------+
   |                     Mobile Robot Programming Toolkit (MRPT)            |
   |                          https://www.mrpt.org/                         |
   |                                                                        |
   | Copyright (c) 2005-2022, Individual contributors, see AUTHORS file     |
   | See: https://www.mrpt.org/Authors - All rights reserved.               |
   | Released under BSD License. See: https://www.mrpt.org/License          |
   +------------------------------------------------------------------------+ */

#include "obs-precomp.h"  // Precompiled headers
//
#include <mrpt/obs/CObservationBeaconRanges.h>
#include <mrpt/serialization/CArchive.h>
#include <mrpt/system/os.h>

using namespace mrpt::obs;
using namespace mrpt::poses;

// This must be added to any CSerializable class implementation file.
IMPLEMENTS_SERIALIZABLE(CObservationBeaconRanges, CObservation, mrpt::obs)

/** Default constructor.
 */
CObservationBeaconRanges::CObservationBeaconRanges()
	: sensedData(), auxEstimatePose()
{
}

uint8_t CObservationBeaconRanges::serializeGetVersion() const { return 3; }
void CObservationBeaconRanges::serializeTo(
	mrpt::serialization::CArchive& out) const
{
	uint32_t i, n;
	// The data
	out << minSensorDistance << maxSensorDistance << stdError;
	n = sensedData.size();
	out << n;
	for (i = 0; i < n; i++)
		out << sensedData[i].sensorLocationOnRobot
			<< sensedData[i].sensedDistance << sensedData[i].beaconID;
	out << auxEstimatePose;
	out << sensorLabel << timestamp;
}

void CObservationBeaconRanges::serializeFrom(
	mrpt::serialization::CArchive& in, uint8_t version)
{
	switch (version)
	{
		case 0:
		case 1:
		case 2:
		case 3:
		{
			uint32_t i, n, id;

			// The data
			in >> minSensorDistance >> maxSensorDistance >> stdError;

			in >> n;
			sensedData.resize(n);
			for (i = 0; i < n; i++)
			{
				in >> sensedData[i].sensorLocationOnRobot >>
					sensedData[i].sensedDistance;
				in >> id;
				sensedData[i].beaconID = id;
			}

			if (version >= 1) in >> auxEstimatePose;

			if (version >= 2) in >> sensorLabel;
			else
				sensorLabel = "";

			if (version >= 3) in >> timestamp;
			else
				timestamp = INVALID_TIMESTAMP;
		}
		break;
		default: MRPT_THROW_UNKNOWN_SERIALIZATION_VERSION(version);
	};
}

void CObservationBeaconRanges::debugPrintOut()
{
	printf("[CObservationBeaconRanges::debugPrintOut] Dumping:\n");
	printf(
		"[CObservationBeaconRanges::debugPrintOut] minSensorDistance:\t%f\n",
		minSensorDistance);
	printf(
		"[CObservationBeaconRanges::debugPrintOut] maxSensorDistance:\t%f:\n",
		maxSensorDistance);
	printf(
		"[CObservationBeaconRanges::debugPrintOut] stdError:\t%f\n", stdError);
	printf(
		"[CObservationBeaconRanges::debugPrintOut] %u ranges:\n",
		static_cast<unsigned>(sensedData.size()));

	size_t i, n = sensedData.size();
	for (i = 0; i < n; i++)
		printf(
			"[CObservationBeaconRanges::debugPrintOut] \tID[%u]: %f\n",
			sensedData[i].beaconID, sensedData[i].sensedDistance);
}

/*---------------------------------------------------------------
					 getSensorPose
 ---------------------------------------------------------------*/
void CObservationBeaconRanges::getSensorPose(CPose3D& out_sensorPose) const
{
	if (!sensedData.empty())
		out_sensorPose = CPose3D(sensedData[0].sensorLocationOnRobot);
	else
		out_sensorPose = CPose3D(0, 0, 0);
}

/*---------------------------------------------------------------
					 setSensorPose
 ---------------------------------------------------------------*/
void CObservationBeaconRanges::setSensorPose(const CPose3D& newSensorPose)
{
	size_t i, n = sensedData.size();
	if (n)
		for (i = 0; i < n; i++)
			sensedData[i].sensorLocationOnRobot = CPoint3D(newSensorPose);
}

/*---------------------------------------------------------------
					 getSensedRangeByBeaconID
 ---------------------------------------------------------------*/
float CObservationBeaconRanges::getSensedRangeByBeaconID(int32_t beaconID)
{
	for (auto& i : sensedData)
		if (i.beaconID == beaconID) return i.sensedDistance;
	return 0;
}

void CObservationBeaconRanges::getDescriptionAsText(std::ostream& o) const
{
	using namespace std;
	CObservation::getDescriptionAsText(o);

	o << "Auxiliary estimated pose (if available): " << auxEstimatePose << endl;

	o << format("minSensorDistance=%f m\n", minSensorDistance);
	o << format("maxSensorDistance=%f m\n", maxSensorDistance);
	o << format("stdError=%f m\n\n", stdError);

	o << format(
		"There are %u range measurements:\n\n", (unsigned)sensedData.size());

	o << "  BEACON   RANGE     SENSOR POSITION ON ROBOT \n";
	o << "------------------------------------------------\n";
	for (const auto& d : sensedData)
	{
		o << format(
			"   %i      %.04f      (%.03f,%.03f,%.03f)\n", (int)d.beaconID,
			d.sensedDistance, d.sensorLocationOnRobot.x(),
			d.sensorLocationOnRobot.y(), d.sensorLocationOnRobot.z());
	}
}

std::string CObservationBeaconRanges::exportTxtHeader() const
{
	return "[BEACON_ID  RANGE  SENSOR_LOCATION_ON_ROBOT] x N \n";
}
std::string CObservationBeaconRanges::exportTxtDataRow() const
{
	std::stringstream o;
	for (const auto& d : sensedData)
	{
		o << format(
			"   %i      %.04f      %.03f %.03f %.03f", (int)d.beaconID,
			d.sensedDistance, d.sensorLocationOnRobot.x(),
			d.sensorLocationOnRobot.y(), d.sensorLocationOnRobot.z());
	}
	return o.str();
}
