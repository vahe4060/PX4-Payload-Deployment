#include <containers/IntrusiveSortedList.hpp>


struct Payload: public IntrusiveSortedListNode<Payload *>
{
	Payload(unsigned index, float weight, float area_x, float area_y,
		float drag_coef, float altitude,
		double destination_lat, double destination_lon,
		int pwm_id, int pwm_open_freq, int pwm_close_freq)
		: IntrusiveSortedListNode<Payload *>()
		, _index(index)
		, _weight(weight)
		, _area_x(area_x)
		, _area_y(area_y)
		, _drag_coef(drag_coef)
		, _altitude(altitude)
		, _destination_lat(destination_lat)
		, _destination_lon(destination_lon)
		, _pwm_id(pwm_id)
		, _pwm_open_freq(pwm_open_freq)
		, _pwm_close_freq(pwm_close_freq)
	{}
	inline bool operator<=(const Payload &other) const {
		return _index <= other._index;
	}
	unsigned _index;
	float _weight;
	float _area_x;
	float _area_y;
	float _drag_coef;
	float _altitude;
	double _destination_lat;
	double _destination_lon;
	int _pwm_id;
	int _pwm_open_freq;
	int _pwm_close_freq;
};
