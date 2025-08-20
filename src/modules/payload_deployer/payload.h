

struct Payload
{
	Payload (float weight, float area_x, float area_y,
		float drag_coef, float altitude,
		double destination_lat, double destination_lon)
		: _weight(weight)
		, _area_x(area_x)
		, _area_y(area_y)
		, _drag_coef(drag_coef)
		, _altitude(altitude)
		, _destination_lat(destination_lat)
		, _destination_lon(destination_lon)
	{}
	float _weight;
	float _area_x;
	float _area_y;
	float _drag_coef;
	float _altitude;
	double _destination_lat;
	double _destination_lon;
};
