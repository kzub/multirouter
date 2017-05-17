#ifndef TESTCPP_H
#define TESTCPP_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fstream>
#include <ios>
#include <iostream>
#include <set>
#include <string>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

struct DayChange;

// Time  ---------------------------------------------------------
struct Time {
 private:
  int16_t value = 0;
  int16_t convert(const char* c);

 public:
  Time() = default;
  Time(char* time, uint16_t width);
  Time(decltype(Time::value) v);
  Time(std::string v);

  std::string toString() const;
  decltype(Time::value) getMinutes();
  void add(decltype(Time::value) v);
  bool operator<(const Time& t) const;
  bool operator>(const Time& t) const;
  bool operator==(const Time& t) const;
  Time operator-(const Time& t) const;
  Time operator+(const Time& t) const;
  void operator+=(const int16_t d);
  void operator+=(const Time& t);
  const static decltype(Time::value) dayMinutes = 60 * 24;
  const static decltype(Time::value) hourMinutes = 60;
  friend struct Node;
  friend struct Compare;
};

// Date  ---------------------------------------------------------
uint8_t static const normalYear[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
uint8_t static const leapYear[] = {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
uint8_t static const maxMonth = 12;

struct Date {
  Date() = default;
  Date(char* date, uint16_t width);
  Date(std::string);
  std::string toString() const;
  bool operator<(const Date& d) const;
  bool operator>(const Date& d) const;
  bool operator==(const Date& d) const;
  bool operator==(const char* d) const;

  void operator+=(const int8_t d);
  void operator+=(const DayChange& d);

 private:
  uint32_t value = 0;
  uint32_t convert(const char*) const;
  void minusDay(const uint8_t d);
  void plusDay(const uint8_t d);
};

// Airport -----------------------------------------------------------
struct Airport {
 private:
  uint32_t value = 0;

 public:
  Airport() = default;
  Airport(const char* code, uint16_t width);
  Airport(std::string code);
  std::string toString() const;

  friend bool operator==(const Airport& a, const Airport& b);
  friend struct Hash;
  friend struct Compare;
};

bool operator==(const Airport& a, const Airport& b);

// Airline -----------------------------------------------------------
struct Airline {
 private:
  uint16_t value = 0;

 public:
  Airline() = default;
  Airline(char* code, uint16_t width);
  std::string toString() const;
};

// Flight number ---------------------------------------------------------
struct FlightNumber {
  FlightNumber() = default;
  FlightNumber(char* code, uint16_t len);
  std::string toString() const;

 private:
  uint16_t value = 0;
};

// Day change ---------------------------------------------------------
struct DayChange {
 private:
  int8_t value = 0;

 public:
  DayChange() = default;
  DayChange(char* code, uint16_t len);
  std::string toString() const;
  decltype(DayChange::value) get() const;
  operator bool() const;
  friend struct Date;
};

// Node -------------------------------------
struct Flight;
struct Node;

Node& getWorldNode(Airport airport);

// Flight -------------------------------------
struct Flight {
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

  Flight(Node& from, Node& to, Airline& airline, FlightNumber& flightNo, Time& startTime,
         Time& endTime, DayChange& dayChange, Date& valid_from, Date& valid_till);
  void print(std::string spacer = "") const;
  std::string toString() const;
  bool isValid(const Date& departureDate) const;
  friend struct Compare;
};

// World -------------------------------------
struct Hash {
  std::size_t operator()(Airport const& obj) const;
  std::size_t operator()(Node const& obj) const;
  std::size_t operator()(Node const* obj) const;
  std::hash<decltype(Airport::value)> airport_hash;
};

struct Compare {
  bool operator()(const Flight& lhs, const Flight& rhs) const;
  std::less<decltype(decltype(Flight::endTime)::value)> endTimeLess;
};

struct Node {
  Airport airport;
  decltype(Time::value) gmt = 0;
  std::set<const Flight, Compare> flights;
  std::unordered_set<const Node*, Hash> links;
};

bool operator==(const Node& a, const Node& b);

struct Route {
  std::vector<const Node*> nodes;
  std::vector<Flight> flights;
  Time duration;
  Time journey;
};
// using Route = std::vector<const Node*>;

#endif