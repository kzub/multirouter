#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fstream>
#include <ios>
#include <iostream>
#include <string>
#include <string>
#include <unordered_map>
#include <vector>

#include "memusage.c"

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

// Date  ---------------------------------------------------------
struct Time {
 private:
  int16_t value;

 public:
  Time() = default;
  Time(char* time, uint16_t width);
  Time(decltype(Time::value) v);
  std::string toString();
  decltype(Time::value) getMinutes();
  void add(decltype(Time::value) v);
  const static decltype(Time::value) dayMinutes = 60 * 24;
  const static decltype(Time::value) hourMinutes = 60;
};

Time::Time(char* time, uint16_t width) {
  if (width != 8) {
    std::cout << "bad time:" << time << std::endl;
    throw "bad time";
  }
  char buf[3];
  buf[0] = time[0];
  buf[1] = time[1];
  decltype(Time::value) hours = std::atoi(buf);
  buf[0] = time[3];
  buf[1] = time[4];
  decltype(Time::value) minutes = std::atoi(buf);

  value = hours * 60 + minutes;
}

Time::Time(decltype(Time::value) v) {
  value = v;
}

decltype(Time::value) Time::getMinutes() {
  return value;
}

void Time::add(decltype(Time::value) v) {
  value += v;
}

std::string Time::toString() {
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

// Date  ---------------------------------------------------------
struct Date {
  Date() = default;
  Date(char* date, uint16_t width);
  std::string toString();

 private:
  uint32_t value;
};

Date::Date(char* date, uint16_t width) {
  if (width != 10 || date[2] != '/' || date[5] != '/') {
    std::cout << "bad date:" << date << std::endl;
    throw "bad date";
  }
  char t[9];
  t[0] = date[6];
  t[1] = date[7];
  t[2] = date[8];
  t[3] = date[9];
  t[4] = date[3];
  t[5] = date[4];
  t[6] = date[0];
  t[7] = date[1];

  value = std::stol(t);
}

std::string Date::toString() {
  auto year = value / 10000;
  auto month = value / 100 - year * 100;
  auto day = value - year * 10000 - month * 100;

  return zeroPad(year, 4) + "/" + zeroPad(month, 2) + "/" + zeroPad(day, 2);
}

struct Airport {
  uint32_t value;
};
bool operator==(const Airport& a, const Airport& b) {
  return a.value == b.value;
}

struct Airline {
  uint16_t value;
};

// Flight number ---------------------------------------------------------
struct FlightNumber {
  FlightNumber() = default;
  FlightNumber(char* code, uint16_t len);
  std::string toString();

 private:
  uint16_t value;
};

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

std::string FlightNumber::toString() {
  auto fns = std::to_string(value);
  return zeroPad(fns, 4);
}

// Day change ---------------------------------------------------------
struct DayChange {
 private:
  int8_t value = 0;

 public:
  DayChange() = default;
  DayChange(char* code, uint16_t len);
  std::string toString();
  decltype(DayChange::value) get();
};

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

std::string DayChange::toString() {
  if (value == 0) {
    return " ";
  }
  if (value > 0) {
    return "+";
  }
  return "-";
};

decltype(DayChange::value) DayChange::get() {
  return value;
}

//-------------------------------------
struct Link;
struct Node;

struct Hash {
  std::size_t operator()(Airport const& obj) const {
    return int32hash(obj.value);
  }
  std::hash<uint32_t> int32hash{};
};

std::unordered_map<Airport, Node, Hash> world;

struct Node {
  Airport airport;
  int8_t timezone = 0;  // TODO
  std::vector<Link> links;
};

struct Link {
  Node& from;
  Node& to;

  Airline airline;
  FlightNumber flightNo;
  Time startTime;
  Time endTime;
  Time duration;
  DayChange dayChange;
  Date valid_from;
  Date valid_till;

  Link(Airport& from, Airport& to, Airline& airline, FlightNumber& flightNo, Time& startTime,
       Time& endTime, DayChange& dayChange, Date& valid_from, Date& valid_till);
  void print();
};

Node& getWorldNode(Airport airport) {
  if (world.count(airport) == 0) {
    auto& node = world[airport];
    node.airport = airport;
    return node;
  }
  return world[airport];
}

void addWorldLink(Link& link) {
  world[link.from.airport].links.push_back(link);
}

Link::Link(Airport& _from, Airport& _to, Airline& _airline, FlightNumber& _flightNo,
           Time& _startTime, Time& _endTime, DayChange& _dayChange, Date& _valid_from,
           Date& _valid_till)
    : from{getWorldNode(_from)},
      to{getWorldNode(_to)},
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
  auto timezone_shift = to.timezone * Time::hourMinutes - from.timezone * Time::hourMinutes;
  duration = Time(abs(end - start + dcm + timezone_shift));
}

Airline toAirlineInt(char* code, uint8_t width) {
  if (width == 3) {
  } else if (width != 2) {
    std::cout << "bad line (ac):" << code << std::endl;
    throw "bad airline code";
  }

  return {decltype(Airline::value)(((int)code[0] << 8) + (int)code[1])};
}

std::string fromAirlineInt(Airline code) {
  return std::string{(char)(code.value >> 8)} + std::string{(char)(code.value & 0x0000FFFF)};
}

Airport toAirportInt(char* code, uint16_t width) {
  if (width != 3) {
    std::cout << "bad point:" << code << std::endl;
    throw "bad airport code";
  }

  return {decltype(Airport::value)(((int)code[0] << 16) + ((int)code[1] << 8) + (int)code[2])};
}

std::string fromAirportInt(Airport code) {
  return std::string{(char)(code.value >> 16)} + std::string{(char)(code.value >> 8)} +
         std::string{(char)(code.value & 0x0000FFFF)};
}

void Link::print() {
  std::cout << fromAirlineInt(airline) << " " << flightNo.toString() << " " << startTime.toString()
            << "[" << fromAirportInt(from.airport) << "-" << fromAirportInt(to.airport) << "]"
            << endTime.toString() << dayChange.toString() << " dur:" << duration.toString()
            << " valid:[" << valid_from.toString() << " - " << valid_till.toString() << "]"
            << std::endl;
}

int main() {
  auto filename = "./test.csv";
  // auto filename = "../data/full_schedule.csv";

  std::ifstream istrm(filename, std::ios::in);
  if (!istrm.is_open()) {
    std::cout << "failed to open " << filename << '\n';
    return -1;
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
          airline = toAirlineInt(buf_start, width);
        } else if (column == 1) {
          flightNo = FlightNumber(buf_start, width);
        } else if (column == 5) {
          valid_from = Date(buf_start, width);  // valid from
        } else if (column == 6) {
          valid_till = Date(buf_start, width);  // valid to
        } else if (column == 14) {
          from = toAirportInt(buf_start, width);
        } else if (column == 15) {
          startTime = Time(buf_start, width);  // time start
        } else if (column == 17) {
          to = toAirportInt(buf_start, width);
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
    Link link(from, to, airline, flightNo, startTime, endTime, dayChange, valid_from, valid_till);
    link.print();
    addWorldLink(link);
  }

  for (auto k : world) {
    std::cout << fromAirportInt(k.first) << std::endl;
  }

  std::cout << "Line count:" << lines << std::endl;
  std::cout << "World size:" << world.size() << std::endl;
  std::cout << "RSS: " << getPeakRSS() << std::endl;

  // std::cout << "Link:" << sizeof(Link) << " Link2:" << sizeof(Link2) << std::endl;
  return 0;
}
