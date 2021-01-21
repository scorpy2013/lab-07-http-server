// Copyright 2020 Your Name <your_email>

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/config.hpp>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include "JsonArray.hpp"
#include "Suggestions.hpp"

namespace beast = boost::beast;    // from <boost/beast.hpp>
namespace http = beast::http;      // from <boost/beast/http.hpp>
namespace net = boost::asio;       // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;  // from <boost/asio/ip/tcp.hpp>
std::string DoJson(const json& Json) {
  std::stringstream ss;
  if (Json.is_null())
    ss << "JSON file is empty!!!";
  else
    ss << std::setw(4) << Json;
  return ss.str();
}

template <class Text, class Distributor, class Sending>
void httpRequest(
    http::request<Text, http::basic_fields<Distributor>>&& distr,
    Sending&& send, const std::shared_ptr<std::timed_mutex>& mutex,
    const std::shared_ptr<Suggestions>& collection) {
  auto const bad_request = [&distr](beast::string_view why) {
    http::response<http::string_body> result{http::status::bad_request,
                                             distr.version()};
    result.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    result.set(http::field::content_type, "text/html");
    result.keep_alive(distr.keep_alive());
    result.body() = std::string(why);
    result.prepare_payload();
    return result;
  };

  auto const not_found = [&distr](beast::string_view target) {
    http::response<http::string_body> result{http::status::not_found,
                                             distr.version()};
    result.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    result.set(http::field::content_type, "text/html");
    result.keep_alive(distr.keep_alive());
    result.body() = std::string(target) + " was not found.";
    result.prepare_payload();
    return result;
  };

  if (distr.method() == http::verb::get) {
    return send(bad_request("This is first request."));
  }
  if (distr.method() != http::verb::post &&
      distr.method() != http::verb::head) {
    return send(bad_request("Unknown HTTP-method"));
  }

  if (distr.target() != "/v1/api/suggest") {
    return send(not_found(distr.target()));
  }

  json Json;
  try {
    Json = json::parse(distr.body());
  } catch (std::exception& exc) {
    return send(bad_request(exc.what()));
  }
  boost::optional<std::string> input;
  try {
    input = Json.at("input").get<std::string>();
  } catch (std::exception& e) {
    return send(
        bad_request(R"(Correct JSON-file: {"input": "<user-message>"})"));
  }
  if (!input.has_value()) {
    return send(
        bad_request(R"(Correct JSON-file: {"input": "<user-message>"})"));
  }

  mutex->lock();
  auto result = collection->DoSuggest(input.value());
  mutex->unlock();
  http::string_body::value_type body = DoJson(result);
  auto const size = body.size();
  if (distr.method() == http::verb::head) {
    http::response<http::empty_body> res{http::status::ok, distr.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "application/json");
    res.content_length(size);
    res.keep_alive(distr.keep_alive());
    return send(std::move(res));
  }

  http::response<http::string_body> result1{
      std::piecewise_construct, std::make_tuple(std::move(body)),
      std::make_tuple(http::status::ok, distr.version())};
  result1.set(http::field::server, BOOST_BEAST_VERSION_STRING);
  result1.set(http::field::content_type, "application/json");
  result1.content_length(size);
  result1.keep_alive(distr.keep_alive());
  return send(std::move(result1));
}
void fail(beast::error_code error, char const* str) {
  std::cerr << str << ": " << error.message() << "\n";
}

template <class Stream>
struct send_lambda {
  Stream& stream_;
  bool& close_;
  beast::error_code& ec_;

  explicit send_lambda(Stream& stream, bool& close, beast::error_code& error)
      : stream_(stream), close_(close), ec_(error) {}

  template <bool isRequest, class Body, class Fields>
  void operator()(http::message<isRequest, Body, Fields>&& msg) const {
    close_ = msg.need_eof();
    http::serializer<isRequest, Body, Fields> sr{msg};
    http::write(stream_, sr, ec_);
  }
};

void do_session(net::ip::tcp::socket& socket,
                const std::shared_ptr<Suggestions>& collection,
                const std::shared_ptr<std::timed_mutex>& mutex) {
  bool close = false;
  beast::error_code errorCode;
  beast::flat_buffer buffer;
  send_lambda<tcp::socket> lambda{socket, close, errorCode};
  for (;;) {
    http::request<http::string_body> req;
    http::read(socket, buffer, req, errorCode);
    if (errorCode == http::error::end_of_stream) break;
    if (errorCode) return fail(errorCode, "Problem in reading");
    httpRequest(std::move(req), lambda, mutex, collection);
    if (errorCode) return fail(errorCode, "Problem in writing");
  }
  socket.shutdown(tcp::socket::shutdown_send, errorCode);
}

void Regeneration(const std::shared_ptr<JsonArray>& storage,
                  const std::shared_ptr<Suggestions>& suggestions,
                  const std::shared_ptr<std::timed_mutex>& mutex) {
  for (;;) {
    mutex->lock();
    storage->ReadJson();
    suggestions->Update(storage->GetMemory());
    mutex->unlock();
    std::cout << "Updating was successful!" << std::endl;
    std::this_thread::sleep_for(std::chrono::operator""min(15));
  }
}
int Start(int argc, char* argv[]) {
  std::shared_ptr<std::timed_mutex> mutex =
      std::make_shared<std::timed_mutex>();
  std::shared_ptr<JsonArray> storage = std::make_shared<JsonArray>(
      "/home/alexscorpy/Documents/АЯ/lab-07-http-server/suggestions.json");
  std::shared_ptr<Suggestions> suggestions = std::make_shared<Suggestions>();
  try {
    if (argc != 3) {
      std::cerr << "Usage: suggestion_server <address> <port>\n"
                << "Example:\n"
                << "    http-server-sync 0.0.0.0 8080\n";
      return EXIT_FAILURE;
    }
    auto const address = net::ip::make_address(argv[1]);
    auto const port = static_cast<uint16_t>(std::atoi(argv[2]));

    net::io_context ioContext{1};

    tcp::acceptor acceptor{ioContext, {address, port}};

    std::thread{Regeneration, storage, suggestions, mutex}.detach();
    for (;;) {
      tcp::socket socket{ioContext};

      acceptor.accept(socket);

      std::thread{std::bind(&do_session, std::move(socket),
                            suggestions, mutex)}.detach();
    }
  } catch (std::exception& exc) {
    std::cerr << exc.what() << '\n';
    return EXIT_FAILURE;
  }
}
// Using: ./cmake-build-debug/tests 0.0.0.0 8080
 int main(int argc, char* argv[]) {
  return Start(argc, argv);
}
