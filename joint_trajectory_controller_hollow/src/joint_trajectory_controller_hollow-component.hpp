/*****************************************************************************
  File: joint_trajectory_controller_hollow-component.hpp

  Version: 1.0
  Author: Carlos Faria <carlosfaria89@gmail.com>
  Maintainer: Carlos Faria <carlosfaria89@gmail.com>

  Copyright (C) 2017 Carlos André de Oliveira Faria. All rights reserved.

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *****************************************************************************/

#ifndef OROCOS_JOINT_TRAJECTORY_CONTROLLER_HOLLOW_COMPONENT_HPP
#define OROCOS_JOINT_TRAJECTORY_CONTROLLER_HOLLOW_COMPONENT_HPP

#include "joint_trajectory_controller_hollow_types.hpp"
#include "joint_trajectory_controller_hollow-errorcode.hpp"
#include "point_to_point_trajectory_generator.hpp"
#include "motion_through_points_trajectory_generator.hpp"

#include <rtt/os/Semaphore.hpp>
#include <rtt/RTT.hpp>

class JointTrajectoryControllerHollow : public RTT::TaskContext {
    /*!
     * \brief The MOTION_TYPE enum Specifies the working mode of the component.
     */
    enum JTC_MODE {
        STOP = 0, //!< No motion
        PTP =  1, //!< Trajectory Following Control Point-to-point
        MTP =  2, //!< Trajectory Following Control Motion-through-points
    };

public:
    JointTrajectoryControllerHollow(const std::string &component_name);
    bool configureHook();
    bool startHook();
    void updateHook();
    void stopHook();
    void cleanupHook();

private:
    double m_dt; //!< Cycle time
    //This variable is true if the trajectory to execute is formed by several via-points (>2), and false if the trajectory only specifies the final pose
    bool m_moving;
    JTC_MODE m_mode;
    JTCErrorCode m_ec;
    //! Binary semaphore to synchronyse the execution, considering interrupt calls
    std::shared_ptr<RTT::os::Semaphore> m_sem;

    std::vector<double> m_q_curr;    //size 7
    std::vector<double> m_v_curr;    //size 7
    std::vector<double> m_q_next;    //size 7
    std::vector<double> m_v_next;    //size 7

    std::vector<std::vector<double> > m_q_tar; //size N of 7-size vectors
    std::vector<double> m_t_tar;     //size N

    std::shared_ptr<PointToPointTrajectoryGenerator> PointToPoint;
    std::shared_ptr<MotionThroughPointsTrajectoryGenerator> MotionThroughPoints;

protected:
    //Attributes
    unsigned int m_nj; //!< Number of joints
    double m_max_free_vel; //!< Maximum allowed free motion velocity in joint space
    double m_max_free_acc; //!< Maximum allowed free motion acceleration in joint space
    double m_max_free_jrk; //!< Maximum allowed free motion jerk in joint space
    double m_max_tf_vel; //!< Maximum allowed trajectory following velocity in joint space
    double m_max_tf_acc; //!< Maximum allowed trajectory following acceleration in joint space
    double m_max_tf_jrk; //!< Maximum allowed trajectory following jerk in joint space
    std::vector<double> m_lim; //!< Joint Limits
    double m_tolerance; //!< Calculations tolerance

private:
    /*!
     * \brief StopMotion Stops robot motion and change the robot state to STOP.
     */
    void StopMotion();

private: //OFFLINE Trajectory Generation
    /*!
    * \brief InstantMotion Moves instantly the hollow robot to the target position.
    * \param target_joints Target joint positions to reach.
    * \return Success.
    */
    bool InstantMotion(const std::vector<double>& target_joints);
    /*!
     * \brief SetPointToPoint Starts a new position-based motion from the current to the target joint values.
     * The motion is phase-synchronized if possible, and guaranteed time-synchronized such that, all joints finish the motion
     * synchronously. The motion follows the specified velocity and acceleration limits defined.
     * \param target_joints Target joint positions to reach.
     * \param rel_vel Relative velocity to the default value.
     * \return Success.
     */
    bool SetPointToPoint(const std::vector<double>& target_joints, const double rel_vel);
    /*!
     * \brief SetMotionThroughPoints Starts a new position-based motion throught the points (joint positions) specified.
     * The motion is time-synchronized such that, all joints pass synchronously through each via-point.
     * \param target_joints Trajectory via-points.
     * \param m_target_times Time associated to via-points.
     * \return Success.
     */
    bool SetMotionThroughPoints(const vector<vector<double> >& target_joints, const vector<double>& m_target_times);

    /*!
     * \brief HandleReflexxesError This method was specially created to handle Reflexxes error code system.
     * Since Reflexxes does not provide a readable output message, it is up to the developer to relate the returned
     * error codes to messages. This messages are then sent through the error port. Other messages are treated normally.
     * \param code as read from the ReflexxesAPI enum.
     */
    void HandleReflexxesError(int code);

protected:
    //Fine joint interpolation trajectory generation - High rate
    RTT::InputPort< std::vector<double> > inport_current_joint_positions;
    RTT::InputPort< std::vector<double> > inport_current_joint_velocities;

    RTT::OutputPort< std::vector<double> > outport_next_joint_positions;
    RTT::OutputPort< RTT::error_msg > outport_error_msg;
};

#endif //OROCOS_JOINT_TRAJECTORY_CONTROLLER_COMPONENT_HPP
