# PX4 Drone Autopilot for Payload Deployment

## How to Use

### Clone the repository
`git clone git@github.com:vahe4060/PX4-Payload-Deployment.git --recursive`

### Setup PX4 dev environment
`(bash): source ./Tools/setup/ubuntu.sh`


### Simulator issues

In case Gazebo does not start or show picture, change its engine to a tested one that works on your machine.

`(bash): export PX4_GZ_SIM_RENDER_ENGINE=ogre`

If it fails too, try using `jmavsim` simulator instead.


## Supported Hardware

For the most up to date information, please visit [PX4 User Guide > Autopilot Hardware](https://docs.px4.io/main/en/flight_controller/).


<a href="https://www.dronecode.org/" style="padding:20px" ><img src="https://dronecode.org/wp-content/uploads/sites/24/2020/08/dronecode_logo_default-1.png" alt="Dronecode Logo" width="110px"/></a>
<div style="padding:10px">&nbsp;</div>
