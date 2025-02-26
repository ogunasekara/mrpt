/* +------------------------------------------------------------------------+
   |                     Mobile Robot Programming Toolkit (MRPT)            |
   |                          https://www.mrpt.org/                         |
   |                                                                        |
   | Copyright (c) 2005-2022, Individual contributors, see AUTHORS file     |
   | See: https://www.mrpt.org/Authors - All rights reserved.               |
   | Released under BSD License. See: https://www.mrpt.org/License          |
   +------------------------------------------------------------------------+ */

#pragma once

#include <mrpt/config/CLoadableOptions.h>

#include <string>
#include <vector>

namespace mrpt::graphslam
{
/**\brief Class to monitor the evolution of a statistical quantity.
 *
 * ## Description
 *
 * Keeps track of the last N incoming measurements of the quantity at hand and
 * upon request returns statistical properties of these measurements (mean,
 * median, etc.)
 *
 * Class also contains methods for evaluating incoming measurements (whether to
 * accept or reject those)
 *
 * ### .ini Configuration Parameters </b>
 *
 * \htmlinclude graphslam-engine_config_params_preamble.txt
 *
 * - \b sliding_win_size
 *   + \a Default value : 10
 *   + \a Required      : FALSE
 *   + \a Description   : How many of the latest measurements to take into
 *   account when computing the relevant statistics
 *
 * \ingroup mrpt_graphslam_grp
 */
struct TSlidingWindow : public mrpt::config::CLoadableOptions
{
   public:
	TSlidingWindow(const std::string& name = "window");
	~TSlidingWindow() override = default;
	/**\brief Return the current median value.  */
	double getMedian();
	/**\brief Return the current mean value.  */
	double getMean();
	/**\brief Return the Standard deviation of the current measurement vector*/
	double getStdDev();
	/**\brief Determine whether the incoming measurement is inside the
	 * [-3sigma, +3sigma] boundaries from the current mean value.
	 *
	 * \return True if it's inside the uncertainty boundaries
	 */
	bool evaluateMeasurementInGaussian(double measurement);
	/**\brief Determine whether the incoming measurement is over the current
	 * mean value.
	 *
	 * \return True if it's above the mean
	 */
	bool evaluateMeasurementAbove(double value);
	/**\brief Determine whether the incoming measurement is *less or equal* to
	 * the current mean value.
	 *
	 * \return True if it's <= to the mean
	 */
	bool evaluateMeasurementBelow(double value);
	/**\brief Update the sliding window by appending a new measurement */
	void addNewMeasurement(double measurement);
	/** Resize the window.
	 *
	 * \note Method affects the underlying vector only if the new_size
	 * specified has already been reached
	 */
	void resizeWindow(size_t new_size);
	void loadFromConfigFile(
		const mrpt::config::CConfigFileBase& source,
		const std::string& section) override;
	void dumpToTextStream(std::ostream& out) const override;

	/**\brief Return the size of the window
	 */
	size_t getWindowSize() const;
	/**\brief Check if the window has reached its limit. This limit is set by
	 * the user via the resizeWindow method.
	 *
	 * \sa resizeWindow
	 */
	bool windowIsFull() const;

   private:
	size_t m_win_size = 5;
	std::vector<double> m_measurements_vec;

	/**\brief Name of the TSlidingWindow Instance at hand */
	std::string m_name;

	double m_mean_cached = 0.0f; /**< Cached mean value */
	double m_median_cached = 0.0f; /**< Cached median value */
	double m_std_dev_cached =
		0.0f; /**< Cached version of the standard deviation */
	bool m_mean_updated = false; /**< Is the mean up-to-date? */
	bool m_median_updated = false; /**< Is the median up-to-date? */
	bool m_std_dev_updated =
		false; /**< Is the standard deviation up-to-date? */

	/**\brief flag is raised the first time that
	 * TSlidingWindow::addNewMeasurement is called
	 */
	bool m_is_initialized = false;
};
}  // namespace mrpt::graphslam
