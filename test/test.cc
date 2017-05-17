#include "test.h"
#include <cassert>
#include "memusage.c"
#include "timing.cc"

// Utils  ---------------------------------------------------------
std::string zeroPad(std::string v, uint8_t count) {
  while (v.size() < count) {
    v = "0" + v;
  }
  return v;
}
std::string zeroPad(long iv, uint8_t count) {
  std::string v = std::to_string(iv);
  while (v.size() < count) {
    v = "0" + v;
  }
  return v;
}

// Time  ---------------------------------------------------------
Time::Time(char* time, uint16_t width) {
  if (width != 8) {
    std::cout << "bad time:" << time << std::endl;
    throw "bad time";
  }
  int16_t hours = convert(time);
  int16_t minutes = convert(time + 3);
  value = hours * 60 + minutes;
}

int16_t Time::convert(const char* time) {
  char buf[3] = {0, 0, 0};
  buf[0] = time[0];
  buf[1] = time[1];
  return std::atoi(buf);
}

Time::Time(decltype(Time::value) v) {
  value = v;
}

Time::Time(std::string time) {
  if (time.size() != 8) {
    std::cout << "bad time2:" << time << std::endl;
    throw "bad time2";
  }
  auto hours = convert(time.c_str());
  auto minutes = convert(time.c_str() + 3);
  value = hours * 60 + minutes;
}

decltype(Time::value) Time::getMinutes() {
  return value;
}

void Time::add(decltype(Time::value) v) {
  value += v;
}

std::string Time::toString() const {
  decltype(Time::value) hours = value / 60;
  decltype(Time::value) minutes = value - hours * 60;
  auto h = std::to_string(hours);
  auto m = std::to_string(minutes);
  if (h.size() == 1) {
    h = "0" + h;
  }
  if (m.size() == 1) {
    m = "0" + m;
  }
  return h + ":" + m;
}

bool Time::operator<(const Time& t) const {
  return value < t.value;
}
bool Time::operator>(const Time& t) const {
  return value > t.value;
}
bool Time::operator==(const Time& t) const {
  return value == t.value;
}
void Time::operator+=(const int16_t d) {
  value += d;
}
void Time::operator+=(const Time& t) {
  value += t.value;
}
Time Time::operator-(const Time& t) const {
  return {decltype(Time::value)(value - t.value)};
}
Time Time::operator+(const Time& t) const {
  return {decltype(Time::value)(value + t.value)};
}

// Date  ---------------------------------------------------------
Date::Date(char* date, uint16_t width) {
  if (width != 10 || date[2] != '/' || date[5] != '/') {
    std::cout << "bad date:" << date << std::endl;
    throw "bad date";
  }
  value = convert(date);
}

Date::Date(std::string date) {
  if (date.size() != 10 || date[2] != '/' || date[5] != '/') {
    std::cout << "bad date:" << date << std::endl;
    throw "bad date";
  }
  value = convert(date.c_str());
}

// 20/12/2017
uint32_t Date::convert(const char* date) const {
  char t[] = {0, 0, 0, 0, 0};
  t[0] = date[0];
  t[1] = date[1];
  uint8_t day = std::stol(t);

  t[0] = date[3];
  t[1] = date[4];
  uint8_t month = std::stol(t);

  t[0] = date[6];
  t[1] = date[7];
  t[2] = date[8];
  t[3] = date[9];
  uint16_t year = std::stol(t);

  return (year << 16) + (month << 8) + day;
}

std::string Date::toString() const {
  uint16_t year = value >> 16;
  uint8_t month = value >> 8;
  uint8_t day = value;

  return zeroPad(year, 4) + "/" + zeroPad(month, 2) + "/" + zeroPad(day, 2);
}

bool Date::operator<(const Date& d) const {
  return value < d.value;
}

bool Date::operator>(const Date& d) const {
  return value > d.value;
}
bool Date::operator==(const Date& d) const {
  return value == d.value;
}

bool Date::operator==(const char* d) const {
  return value == convert(d);
}

// Учесть влияние количества месяцев при сложение дат
// Январь   31
// Февраль  28 / 29
// Март    31
// Апрель    30
// Май   31
// Июнь    30
// Июль    31
// Август    31
// Сентябрь    30
// Октябрь   31
// Ноябрь    30
// Декабрь   31
void Date::plusDay(const uint8_t d) {
  value += d;
  uint16_t year = value >> 16;
  uint8_t month = value >> 8;
  uint8_t day = value;
  uint8_t maxDays = normalYear[month];
  if (year % 4 == 0) {
    maxDays = leapYear[month];
  }

  if (day <= maxDays) {
    return;
  }

  day = day - maxDays;
  month++;
  if (month > maxMonth) {
    month = 1;
    year++;
  }
  value = (year << 16) + (month << 8) + day;
}

void Date::minusDay(const uint8_t d) {
  value -= d;
  uint16_t year = value >> 16;
  uint8_t month = value >> 8;
  uint8_t day = value;

  if (day == 0) {
    month--;
  }
  if (month == 0) {
    month = maxMonth;
    year--;
  }

  uint8_t maxDays = normalYear[month];
  if (year % 4 == 0) {
    maxDays = leapYear[month];
  }

  if (day == 0 || day > maxDays) {
    day = maxDays + day;
  }

  value = (year << 16) + (month << 8) + day;
}

void Date::operator+=(const int8_t d) {
  if (d == 0) {
    return;
  }
  if (d > 10) {
    throw "big hops not supported";
  }
  if (d > 0) {
    plusDay(d);
  } else {
    minusDay(abs(d));
  }
}

void Date::operator+=(const DayChange& d) {
  operator+=(d.get());
}

// Airport -----------------------------------------------------------

// used to compare hashes in map
bool operator==(const Airport& a, const Airport& b) {
  return a.value == b.value;
}

Airport::Airport(const char* code, uint16_t width) {
  if (width != 3) {
    std::cout << "bad point:" << std::to_string(width) << " " << code << std::endl;
    throw "bad airport code";
  }
  value = ((int)code[0] << 16) + ((int)code[1] << 8) + (int)code[2];
}

Airport::Airport(std::string code) {
  if (code.size() != 3) {
    std::cout << "bad point:" << std::to_string(code.size()) << " " << code << std::endl;
    throw "bad airport code";
  }
  value = ((int)code[0] << 16) + ((int)code[1] << 8) + (int)code[2];
}

std::string Airport::toString() const {
  return std::string{(char)(value >> 16)} + std::string{(char)(value >> 8)} +
         std::string{(char)(value & 0x0000FFFF)};
}

// Airline -----------------------------------------------------------
Airline::Airline(char* code, uint16_t width) {
  if (width == 3) {
  } else if (width != 2) {
    std::cout << "bad line (ac):" << code << std::endl;
    throw "bad airline code";
  }

  value = ((int)code[0] << 8) + (int)code[1];
}

std::string Airline::toString() const {
  return std::string{(char)(value >> 8)} + std::string{(char)(value & 0x0000FFFF)};
}

// Flight number ---------------------------------------------------------
FlightNumber::FlightNumber(char* code, uint16_t len) {
  if (len > 4) {
    std::cout << "bad fNo:" << code << std::endl;
    throw "toFlightNo error";
  }

  char flno[] = {'0', '0', '0', '0', 0};
  for (int fi = 3; len > 0; --len, --fi) {
    flno[fi] = code[len - 1];
  }
  value = std::stoi(flno);
}

std::string FlightNumber::toString() const {
  auto fns = std::to_string(value);
  return zeroPad(fns, 4);
}

// Day change ---------------------------------------------------------
DayChange::DayChange(char* code, uint16_t width) {
  if (width != 1 && width != 2) {
    throw "daychange error1";
  }
  char symb[3];
  int c = 0;
  do {
    symb[c] = code[c];
  } while (c++ < width);
  value = std::stoi(symb);
  if (value < 0 && width != 2) {
    throw "daychange error2";
  }
  if (value == 0 && code[0] != '0') {
    throw "daychange error3";
  }
}

DayChange::operator bool() const {
  return value != 0;
}

std::string DayChange::toString() const {
  if (value == 0) {
    return " ";
  }
  if (value > 0) {
    return "+";
  }
  return "-";
};

decltype(DayChange::value) DayChange::get() const {
  return value;
}

// Flight -------------------------------------
Flight::Flight(Node& _from, Node& _to, Airline& _airline, FlightNumber& _flightNo, Time& _startTime,
               Time& _endTime, DayChange& _dayChange, Date& _valid_from, Date& _valid_till)
    : from{_from},
      to{_to},
      airline{_airline},
      flightNo{_flightNo},
      startTime{_startTime},
      endTime{_endTime},
      dayChange{_dayChange},
      valid_from{_valid_from},
      valid_till{_valid_till} {
  //
  auto dcm = dayChange.get() * Time::dayMinutes;
  auto start = startTime.getMinutes();
  auto end = endTime.getMinutes();
  auto timezone_shift = from.gmt - to.gmt;
  duration = Time(abs(end - start + dcm + timezone_shift));
}

void Flight::print(std::string spacer) const {
  std::cout << spacer << this->toString() << std::endl;
}

std::string Flight::toString() const {
  return airline.toString() + " " + flightNo.toString() + " " + startTime.toString() + "[" +
         from.airport.toString() + "-" + to.airport.toString() + "]" + endTime.toString() +
         dayChange.toString() + " dur:" + duration.toString() + " valid:[" + valid_from.toString() +
         " - " + valid_till.toString() + "]";
}

bool Flight::isValid(const Date& departureDate) const {
  if (departureDate < valid_from || departureDate > valid_till) {
    return false;
  }
  return true;
}

// bool Flight::isValidFor(const Date& departureDate, const Time& departureTime) const {
//   if (departureDate < valid_from || departureDate > valid_till) {
//     std::cout << " ^1^ ";
//     return false;
//   }
//   if (departureTime > startTime) {
//     std::cout << " ^2^ ";
//     return false;
//   }
//   return true;
// }

std::size_t Hash::operator()(Airport const& obj) const {
  return airport_hash(obj.value);
}
std::size_t Hash::operator()(Node const& obj) const {
  return airport_hash(obj.airport.value);
}
std::size_t Hash::operator()(Node const* obj) const {
  return airport_hash(obj->airport.value);
}

bool Compare::operator()(const Flight& l, const Flight& r) const {
  if (l.from.airport == r.from.airport && l.to.airport == r.to.airport) {
    return l.endTime.value < r.endTime.value;
  }
  if (l.from.airport == r.from.airport) {
    return l.to.airport.value < r.to.airport.value;
  }
  return l.from.airport.value < r.from.airport.value;
}

bool operator==(const Node& a, const Node& b) {
  return a.airport == b.airport;
}

// World -------------------------------------
std::unordered_map<Airport, Node, Hash> world;
std::unordered_map<Airport, Time, Hash> gmt;

Node& getWorldNode(Airport airport) {
  if (world.count(airport) == 0) {
    auto& node = world[airport];
    node.airport = airport;
    if (gmt.count(airport)) {
      node.gmt = gmt[airport].getMinutes();
    } else {
      // std::cout << "WARN no gmt for:" << airport.toString() << std::endl;
    }
    return node;
  }
  return world[airport];
}

void addWorldFlight(const Flight& link) {
  auto& from = link.from.airport;
  auto& node = world[from];
  // std::cout << "addWorldFlight:" << link.from.airport.toString() << " "
  // << link.to.airport.toString() << " " << node.flights.size() << std::endl;
  node.flights.insert(link);
  node.links.insert(&link.to);
}

void fillGmt() {
  auto filename = "gmt.dat";

  std::ifstream istrm(filename, std::ios::in);
  if (!istrm.is_open()) {
    std::cout << "failed to open " << filename << '\n';
    throw "failed to open gmt file";
  }

  char buf[1024];
  long lines = 0;

  while (istrm.getline(buf, sizeof(buf), '\n')) {
    // std::cout << "buf:" << buf << std::endl;
    Airport airport(buf, 3);
    Time offset(std::stoi(buf + 4));
    // std::cout << "GMT:" << airport.toString() << " " << offset.toString() << std::endl;
    gmt[airport] = offset;
  }
}

void makeWorld(bool bigworld) {
  auto filename = "test/test.csv";
  if (bigworld) {
    filename = "data/full_schedule.csv";
  }

  std::ifstream istrm(filename, std::ios::in);
  if (!istrm.is_open()) {
    std::cout << "failed to open " << filename << '\n';
    throw "failed to open world data";
  }

  char buf[1024];
  long lines = 0;

  while (istrm.getline(buf, sizeof(buf), '\n')) {
    // std::cout << "res:" << res << std::endl;
    if (lines++ == 0) {
      continue;
    }

    Airline airline;
    FlightNumber flightNo;
    Time startTime;
    Time endTime;
    DayChange dayChange;
    Date valid_from;
    Date valid_till;
    Airport from;
    Airport to;

    int column = 0;
    long last_i = 0;
    char* buf_start = buf;

    for (long i = 0, j = istrm.gcount(); i < j; ++i) {
      if (buf[i] == ',') {
        uint16_t width = i - last_i;

        if (column == 0) {
          airline = Airline(buf_start, width);
        } else if (column == 1) {
          flightNo = FlightNumber(buf_start, width);
        } else if (column == 5) {
          valid_from = Date(buf_start, width);  // valid from
        } else if (column == 6) {
          valid_till = Date(buf_start, width);  // valid to
        } else if (column == 14) {
          from = Airport(buf_start, width);
        } else if (column == 15) {
          startTime = Time(buf_start, width);  // time start
        } else if (column == 17) {
          to = Airport(buf_start, width);
        } else if (column == 18) {
          endTime = Time(buf_start, width);  // time end
        } else if (column == 20) {
          dayChange = DayChange(buf_start, width);
        }

        column++;
        last_i = i + 1;
        buf_start = buf + last_i;
      }
    }

    Flight link(getWorldNode(from), getWorldNode(to), airline, flightNo, startTime, endTime,
                dayChange, valid_from, valid_till);
    // link.print();
    addWorldFlight(link);
  }

  for (auto k : world) {
    // std::cout << k.first.toString() << std::endl;
  }

  std::cout << "Line count:" << lines << std::endl;
  std::cout << "World size:" << world.size() << std::endl;
  std::cout << "RSS: " << getPeakRSS() << std::endl;
}

void printRoute(const Route& route) {
  std::cout << "ROUTE: ";
  for (const auto& leg : route.nodes) {
    std::cout << leg->airport.toString();
    if (&leg != &route.nodes.back()) {
      std::cout << "-";
    }
  }
  std::cout << " DURATION:" << route.duration.toString();
  std::cout << " JOURNEY:" << route.journey.toString();
  std::cout << std::endl;

  for (const auto& flight : route.flights) {
    std::cout << "          " << flight.toString() << std::endl;
  }
}

void datesTest() {
  Date d1("01/01/2016");
  assert(d1 == "01/01/2016");
  d1 += 1;
  assert(d1 == "02/01/2016");
  d1 += -1;
  assert(d1 == "01/01/2016");
  d1 += -1;
  assert(d1 == "31/12/2015");

  Date d2("31/12/2016");
  d2 += +1;
  assert(d2 == "01/01/2017");
  d2 += -2;
  assert(d2 == "30/12/2016");
  d2 += -5;
  assert(d2 == "25/12/2016");

  Date d3("01/03/2016");
  d3 += -1;
  assert(d3 == "29/02/2016");

  Date d4("01/03/2015");
  d4 += -1;
  assert(d4 == "28/02/2015");
  d4 += +5;
  std::cout << d4.toString() << std::endl;
  assert(d4 == "05/03/2015");

  std::cout << "OK" << std::endl;
}

// MAIN ------------------------------
int main() {
  auto timer = timing::Timer("TEST");
  fillGmt();
  makeWorld(0);
  timer.tick("world");

  // auto ledNode = world[{"LED"}];
  // std::cout << "ledNode:" << ledNode.flights.size() << std::endl;
  // for (auto& flight : ledNode.flights) {
  //   flight.print();
  // }
  // return 0;

  // auto from = Airport("DME");
  std::unordered_set<const Airport, Hash> from;
  from.insert({"DME"});
  from.insert({"SVO"});
  from.insert({"VKO"});

  auto to = Airport("TXL");
  auto departure = Date("01/09/2017");
  auto departureTime = Time("00:00:00");

  std::vector<Route> routes;

  // Нужна карта связей (аэропорт-аэропорт)
  // Первым этапом набираем маршруты
  // Вторым этапом по каждому маршруту идёт проверка рейсов
  Route route;
  // std::cout << "Enter" << std::endl;
  // std::string test;
  // std::cin >> test;

  for (const auto& point : world) {
    if (from.count(point.first)) {
      route.nodes.clear();
      route.nodes.push_back(&point.second);
      // std::cout << "START: " << point.first.toString() << std::endl;
      uint32_t counter = 0;

      for (const auto& node1 : point.second.links) {
        // std::cout << "LEG1 " << std::to_string(counter++) << " "
        //           << std::to_string(point.second.links.size()) << " links:" <<
        //           node1->links.size()
        //           << std::endl;
        // std::cout << "LEG1:" << node1->airport.toString() << std::endl;
        if (node1->airport == to) {
          route.nodes.push_back(node1);
          routes.push_back(route);
          // std::cout << "ROUTE!" << std::endl;
          route.nodes.pop_back();
        }

        for (const auto& node2 : node1->links) {
          // std::cout << "LEG2:" << node2->airport.toString() << std::endl;
          if (node2->airport == to) {
            route.nodes.push_back(node1);
            route.nodes.push_back(node2);
            routes.push_back(route);
            // std::cout << "ROUTE!2" << std::endl;
            route.nodes.pop_back();
            route.nodes.pop_back();
          }
        }
      }
    }
  }

  timer.tick("routes");
  std::vector<Route> good;

  // routes = [MOW MAD TXL], [MOW PAR TXL], [MOW FRA TXL]
  for (auto& route : routes) {
    printRoute(route);
    auto depDate = departure;
    auto minTime = Time("00:00:00");
    bool stop = true;
    Flight* prevFlight = nullptr;

    // route = [MOW MAD TXL]
    auto node = route.nodes.begin();
    for (auto nodeNext = node + 1; nodeNext != route.nodes.cend(); ++node, ++nodeNext) {
      // node->flights == MOW flights [SU0000, SU1000, LH2033] sorted by arrival time
      std::cout << "node: " << (*node)->airport.toString() << std::endl;

      stop = true;
      for (const auto& flight : (*node)->flights) {
        flight.print("  ");
        if (!(flight.to.airport == (*nodeNext)->airport)) {
          std::cout << "  wrong airport " << flight.to.airport.toString() << " "
                    << (*nodeNext)->airport.toString() << std::endl;
          continue;
        }
        if (!flight.isValid(depDate)) {
          std::cout << "  not valid " << flight.to.airport.toString() << " "
                    << (*nodeNext)->airport.toString() << std::endl;
          continue;
        }
        // std::cout << "valid" << std::endl;
        if (flight.startTime < minTime) {
          // std::cout << "stop1" << std::endl;
          continue;
        }

        stop = false;
        // TODO верная работа с датами
        if (prevFlight != nullptr) {
          if (prevFlight->dayChange) {
            route.journey += (24 * 60);
          }
          // std::cout << "NOW:" << flight.startTime.toString()
          // << " PREV:" << prevFlight->endTime.toString() << std::endl;
          route.journey += (flight.startTime - prevFlight->endTime);
        }

        depDate += flight.dayChange;
        minTime = flight.endTime;
        minTime += 60;  // hour delay for connecting flight

        route.duration += flight.duration;
        route.journey += flight.duration;
        // route.journey += 60;
        route.flights.push_back(flight);
        prevFlight = const_cast<Flight*>(&flight);
        std::cout << "  found, go next node with: " << depDate.toString() << " "
                  << minTime.toString() << std::endl;
        break;
      }

      if (stop == true) {
        // std::cout << "stop2" << std::endl;
        break;
      }
    }

    if (stop == false) {
      std::cout << "GOOD ROUTE" << std::endl;
      good.push_back(route);
    }
  }

  timer.tick("flights");

  std::cout << "GOOD:" << std::endl;
  std::sort(good.begin(), good.end(),
            [](const Route& a, const Route& b) { return a.journey < b.journey; });

  for (const auto& route : good) {
    printRoute(route);
  }

  // std::cout << "Flight:" << sizeof(Flight) << " Flight2:" << sizeof(Flight2) << std::endl;
  return 0;
  }
