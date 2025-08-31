/****************************************************************************
 *
 *   Copyright (c) 2022 PX4 Development Team. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name PX4 nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

#include "payload_deployer.h"
#include <limits.h>

inline bool expect_eq(float a, float b, float eps = 1e-5f) {
    return (a > b ? a - b : b - a) < eps;
}

inline bool expect_eq(double a, double b, double eps = 1e-9) {
    return (a > b ? a - b : b - a) < eps;
}


IntrusiveSortedList<Payload *> PayloadDeployer::_payloads;

unsigned PayloadDeployer::_active_item = 0;

PayloadDeployer::PayloadDeployer()
	: ModuleBase<PayloadDeployer>()
	, ModuleParams(nullptr)
	, ScheduledWorkItem(MODULE_NAME, px4::wq_configurations::lp_default)
{
	ScheduleOnInterval(100_ms);
}

/* initialize instance */
int PayloadDeployer::task_spawn(int argc, char *argv[]) {
	PayloadDeployer *instance = new PayloadDeployer();
	if (instance) {
		_object.store(instance);
		_task_id = task_id_is_work_queue;
		return PX4_OK;
	}
	else {
		PX4_ERR("Alloc failed");
	}
	// Cleanup instance in memory and mark this module as invalid to run
	delete instance;
	_object.store(nullptr);
	_task_id = -1;

	return PX4_ERROR;
}

/* add a new payload */
bool PayloadDeployer::add(int size, char *args[]) {
	if (size != 11) {
		PX4_WARN("Usage:");
		printf("payload_deployer add [index: unsigned] [weight(kg): float] [area_x(sqm): float] [area_y(sqm): float] [drag_coef: float]");
		printf(" [alt(m): float] [lat(WGS84): double] [lon(WGS84): double] [pwm_id: int] [pwm_open: int] [pwm_close: int]\n");
		printf("example:\n\tpayload_deployer add 15 0.3 0.007 0.0022 0.42 200 35.143214165 42.55138791202 14 1500 990\n");
		return PX4_OK;
	}
	unsigned	index = (atoi(args[0]) > USHRT_MAX ||
				 atoi(args[0]) <= 0)? 0 : atoi(args[0]);
	int	pwm_id = atoi(args[8]),
		pwm_open_freq = atoi(args[9]),
		pwm_close_freq = atoi(args[10]);
	float	weight = atof(args[1]),
		area_x = atof(args[2]),
		area_y = atof(args[3]),
		drag_coef = atof(args[4]),
		alt = atof(args[5]);
	double	lat = strtod(args[6], NULL),
		lon = strtod(args[7], NULL);
	if (index == 0 || pwm_id == 0 || pwm_open_freq == 0 || pwm_close_freq == 0 ||
	    expect_eq(weight, 0) || expect_eq(area_x, 0) || expect_eq(area_y, 0) ||
	    expect_eq(drag_coef, 0) || expect_eq(alt, 0) || expect_eq(lat, 0) ||
	    expect_eq(lon, 0)) {
		PX4_ERR("Invalid arguments");
		return PX4_ERROR;
	}
	for (const Payload *it: _payloads) {
		if (it->_index == index) {
			PX4_ERR("Payload with index already exists.");
			return PX4_ERROR;
		}
	}
	Payload *new_item = new Payload(index, weight, area_x, area_y, drag_coef, alt,
				  lat, lon, pwm_id, pwm_open_freq, pwm_close_freq);
	if (!new_item)
		PX4_ERR("Allocation error.");
	_payloads.add(new_item);
	return PX4_OK;
}

/* edit an existing payload by its index */
bool PayloadDeployer::edit(int size, char *args[]) {
	if (size != 3) {
		PX4_WARN("Usage:");
		printf("payload_deployer edit INDEX FIELD_NAME NEW_VALUE\n\
example:\n\
\tpayload_deployer edit 15 area_x 0.0048\n");
		return PX4_OK;
	}

	unsigned	index = (atoi(args[0]) > USHRT_MAX ||
				 atoi(args[0]) <= 0)? 0 : atoi(args[0]);
	Payload *item = nullptr;
	for (Payload *it: _payloads) {
		if (it->_index == index) {
			item = it; break;
		}
	}
	if (!item) {
		PX4_ERR("Couldn't find item with specified index.");
		return PX4_ERROR;
	}
	auto set_val = [] (const char *name, auto &address, auto parse_func, auto validate_func) {
		auto new_val = parse_func();
		if (!validate_func(new_val)) {
			PX4_ERR("Invalid value specified for field");
			return PX4_ERROR;
		}
		address = new_val;
		return PX4_OK;
	};
	if (strcmp(args[1], "index") == 0) {
		return set_val("index", item->_index,
				[&args]() {
					return (unsigned)((atoi(args[2]) > USHRT_MAX ||
							   atoi(args[2]) <= 0)? 0 : atoi(args[2]));
				},
				[](unsigned val) {
					for (Payload *it: _payloads) {
						if (it->_index == val) {
							PX4_ERR("index already in use.");
							return false;
						}
					}
					return val != 0;
				});
	} else if (strcmp(args[1], "weight") == 0) {
		return set_val("weight", item->_weight,
				[&args]() { return atof(args[2]);},
				[](float val) { return !(expect_eq(val, 0) || val < 0); });
	} else if (strcmp(args[1], "area_x") == 0) {
		return set_val("area_x", item->_area_x,
				[&args]() { return atof(args[2]);},
				[](float val) { return !(expect_eq(val, 0) || val < 0); });
	} else if (strcmp(args[1], "area_y") == 0) {
		return set_val("area_y", item->_area_y,
				[&args]() { return atof(args[2]);},
				[](float val) { return !(expect_eq(val, 0) || val < 0); });
	} else if (strcmp(args[1], "drag_coef") == 0) {
		return set_val("drag_coef", item->_drag_coef,
				[&args]() { return atof(args[2]);},
				[](float val) { return !(expect_eq(val, 0) || val < 0); });
	} else if (strcmp(args[1], "pwm_id") == 0) {
		return set_val("pwm_id", item->_pwm_id,
				[&args]() { return atoi(args[2]);},
				[](int val) { return (val < 0); });
	} else if (strcmp(args[1], "pwm_open") == 0) {
		return set_val("pwm_open", item->_pwm_open_freq,
				[&args]() { return atoi(args[2]);},
				[](int val) { return (val < 0); });
	} else if (strcmp(args[1], "pwm_close") == 0) {
		return set_val("pwm_close", item->_pwm_close_freq,
				[&args]() { return atoi(args[2]);},
				[](int val) { return (val < 0); });
	} else if (strcmp(args[1], "alt") == 0) {
		return set_val("alt", item->_altitude,
				[&args]() { return atof(args[2]);},
				[](float val) { return !(expect_eq(val, 0) || val < 0); });
	} else if (strcmp(args[1], "lat") == 0) {
		return set_val("lat", item->_destination_lat,
				[&args]() { return strtod(args[2], NULL);},
				[](double val) { return !(expect_eq(val, 0) || val < 0); });
	} else if (strcmp(args[1], "lon") == 0) {
		return set_val("lon", item->_destination_lon,
				[&args]() { return strtod(args[2], NULL);},
				[](double val) { return !(expect_eq(val, 0) || val < 0); });
	} else {
		PX4_ERR("Can't find field by specified name.");
		return PX4_ERROR;
	}
	return PX4_OK;
}

/* remove an existing payload by its index */
bool PayloadDeployer::remove(int size, char *args[]) {
	if (size != 1) {
		PX4_WARN("Usage:");
		printf("payload_deployer remove INDEX\n\
example:\n\
\tpayload_deployer remove 15\n");
		return PX4_OK;
	}
	unsigned	index = (atoi(args[0]) > USHRT_MAX ||
				 atoi(args[0]) <= 0)? 0 : atoi(args[0]);
	for (Payload *it: _payloads) {
		if (it->_index == index) {
			_payloads.remove(it);
			PX4_INFO("Payload removed");
			return PX4_OK;
		}
	}
	PX4_WARN("Couldn't find payload with index.\n");
	return PX4_OK;
}

/* test servo of payload by its index*/
bool PayloadDeployer::test_servo(int size, char *args[]) {

	return PX4_OK;
}

/* list added payloads */
bool PayloadDeployer::list() {
	printf("[index]  [weight(kg)]  [area_x(sqm)]  [area_y(sqm)]  [drag_coef]  [pwm_id]  [pwm_open]  [pwm_close]  [alt(m)]  [lat(WGS84)]    [lon(WGS84)]\n");
	for (const Payload *it: _payloads) {
		printf(" %-7d  %-12.5f  %-13.5f  %-13.5f  %-11.5f  %-8d  %-10d  %-11d  %-7.2f  %-12.10lf    %-12.10lf\n",
			it->_index,
			it->_weight,
			it->_area_x,
			it->_area_y,
			it->_drag_coef,
			it->_pwm_id,
			it->_pwm_open_freq,
			it->_pwm_close_freq,
			it->_altitude,
			it->_destination_lat,
			it->_destination_lon);
	}
	return PX4_OK;
}

/* start deployment of payloads, if index is not specified, all payloads will be deployed by their order */
bool PayloadDeployer::launch(int size, char *args[]) {
	if (size > 1) {
		PX4_WARN("Usage:");
		printf("payload_deployer launch [[INDEX]]\n\
example:\n\
\tpayload_deployer launch 15 // launch 1 item\n\
example:\n\
\tpayload_deployer launch // launch all\n");
		return PX4_OK;
	}
	if (_active_item != 0)
		PX4_WARN("Another item is already launched, please cancel it first.\n");
	else if (size == 1) { // launch for specified index only
		unsigned index = (atoi(args[0]) > USHRT_MAX ||
				 atoi(args[0]) <= 0)? 0 : atoi(args[0]);
		const Payload *item{nullptr};
		for (const Payload *it: _payloads)
			if (it->_index == index) {
				item = it;
				break;
			}
		if (!item) {
			PX4_ERR("Invalid index");
			return PX4_ERROR;
		}
		// Generate mission for item
		_active_item = index;
	}
	else if (_payloads.size() == 0) { // launch for all
		PX4_WARN("Nothing to launch.");
	}
	else {
		// generate mission for all
		_active_item = 1;
	}
	return PX4_OK;
}

/* cancel deployment of payloads and stop vehicle where it is */
bool PayloadDeployer::cancel() {
	if (_active_item) {
		// cancel mission
		_active_item = 0;
	}
	return PX4_OK;
}

int PayloadDeployer::custom_command(int argc, char *argv[]) {
	if (argc == 0)
		return print_usage();
	else if (strcmp(argv[0], "add") == 0)
		return add(argc - 1, argv + 1);
	else if (strcmp(argv[0], "edit") == 0)
		return edit(argc - 1, argv + 1);
	else if (strcmp(argv[0], "remove") == 0)
		return remove(argc - 1, argv + 1);
	else if (strcmp(argv[0], "test_servo") == 0)
		return test_servo(argc - 1, argv + 1);
	else if (strcmp(argv[0], "list") == 0)
		return list();
	else if (strcmp(argv[0], "launch") == 0)
		return launch(argc - 1, argv + 1);
	else if (strcmp(argv[0], "cancel") == 0)
		return cancel();
	return print_usage("Unrecognized command");
}

int PayloadDeployer::print_usage(const char *reason) {
	if (reason) {
		PX4_WARN("%s\n", reason);
	}

	PRINT_MODULE_DESCRIPTION(
		R"DESCR_STR(
### Description
Handles payload deployment for each item based on its aerodynamic properties.
)DESCR_STR");
	PRINT_MODULE_USAGE_NAME("payload_deployer", "command");
	PRINT_MODULE_USAGE_COMMAND_DESCR("add [index ...]", "Add a new payload.");
	PRINT_MODULE_USAGE_COMMAND_DESCR("edit [index ...]", "Edit payload properties.");
	PRINT_MODULE_USAGE_COMMAND_DESCR("test_servo [index]", "Test servo open/close functionality.");
	PRINT_MODULE_USAGE_COMMAND_DESCR("remove [index]", "Remove the payload.");
	PRINT_MODULE_USAGE_COMMAND_DESCR("list", "List the payloads.");
	PRINT_MODULE_USAGE_COMMAND_DESCR("launch [[index]]", "launch deployment, if index not specified all will be deployed by their order.");
	PRINT_MODULE_USAGE_COMMAND_DESCR("cancel", "List the payloads.");
	PRINT_MODULE_USAGE_DEFAULT_COMMANDS();
	return PX4_OK;
}

void PayloadDeployer::Run() {
	if (should_exit()) {
		ScheduleClear();
		_vehicle_command_sub.unregisterCallback();
		return;
	}

	if (_parameter_update_sub.updated()) {
		parameter_update_s param_update_dummy;
		_parameter_update_sub.copy(&param_update_dummy);
		parameter_update();
	}
}

void PayloadDeployer::parameter_update()
{
	updateParams();
}

/** @see ModuleBase::print_status() */
int PayloadDeployer::print_status() {
	return 0;
}

int payload_deployer_main(int argc, char *argv[]) {
	return PayloadDeployer::main(argc, argv);
}
