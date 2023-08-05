#pragma once
#include <AL/Common.hpp>

#include <AL/Collections/Array.hpp>

#include <AL/Network/HTTP/Client.hpp>

#include "json.hpp"

namespace N2YO
{
	struct Satellite
	{
		AL::uint32 ID;
		AL::String Name;
	};

	struct SatellitePass
	{
		AL::Timestamp Rise;
		AL::Timestamp Set;
		AL::Float     Elevation;

		struct
		{
			AL::Float Start;
			AL::Float End;
			AL::Float Max;
		} Azimuth;
	};

	struct SatelliteRadioPass
		: public SatellitePass
	{
	};

	struct SatelliteVisiblePass
		: public SatellitePass
	{
		AL::TimeSpan Duration;
		AL::Float    Magnitude;
	};

	typedef AL::Collections::Array<SatelliteRadioPass>   SatelliteRadioPasses;
	typedef AL::Collections::Array<SatelliteVisiblePass> SatelliteVisiblePasses;

	struct SatellitePosition
	{
		AL::Float     RA;
		AL::Float     DEC;
		AL::Timestamp Time;
		AL::Float     Azimuth;
		AL::Float     Latitude;
		AL::Float     Longitude;
		AL::Float     Elevation;
	};

	typedef AL::Collections::Array<SatellitePosition> SatellitePositions;

	struct SatelliteRadioPassContext
	{
		SatelliteRadioPasses Passes;
		Satellite            Satellite;
	};

	struct SatelliteVisiblePassContext
	{
		SatelliteVisiblePasses Passes;
		Satellite              Satellite;
	};

	struct SatellitePositionContext
	{
		SatellitePositions Positions;
		Satellite          Satellite;
	};

	template<typename T>
	struct QueryResult
	{
		T          Result;
		AL::uint32 TransactionCount;
	};

	typedef QueryResult<SatelliteRadioPassContext>   SatelliteRadioPassQueryResult;
	typedef QueryResult<SatelliteVisiblePassContext> SatelliteVisiblePassQueryResult;
	typedef QueryResult<SatellitePositionContext>    SatellitePositionQueryResult;

	class API
	{
		AL::String                key;
		AL::Network::HTTP::Client client;

		API(API&&) = delete;
		API(const API&) = delete;

	public:
		explicit API(AL::String&& key)
			: key(
				AL::Move(key)
			)
		{
		}
		explicit API(const AL::String& key)
			: API(
				AL::String(key)
			)
		{
		}

		// @throw AL::Exception
		SatellitePositionQueryResult GetPositions(AL::uint32 id, AL::Float latitude, AL::Float longitude, AL::Float altitude, AL::uint16 count)
		{
			AL::String params[] =
			{
				"positions",
				AL::ToString(id).GetCString(),
				AL::ToString(latitude).GetCString(),
				AL::ToString(longitude).GetCString(),
				AL::ToString(altitude).GetCString(),
				AL::ToString(count).GetCString()
			};

			return ExecuteQuery<SatellitePositionQueryResult>(params, [](nlohmann::json& json)
			{
				SatellitePositionQueryResult result =
				{
					.Result =
					{
						.Positions = SatellitePositions(json.contains("positions") ? json["positions"].size() : 0),
						.Satellite =
						{
							.ID   = json["info"]["satid"].get<std::uint32_t>(),
							.Name = json["info"]["satname"].get<std::string>().c_str()
						}
					},
					.TransactionCount = json["info"]["transactionscount"].get<std::uint32_t>()
				};

				AL::size_t result_position_index = 0;

				for (auto& position : json["positions"])
				{
					auto& result_position = result.Result.Positions[result_position_index++];

					result_position.RA        = position["ra"].get<float>();
					result_position.DEC       = position["dec"].get<float>();
					result_position.Time      = AL::Timestamp::FromSeconds(position["timestamp"].get<float>());
					result_position.Azimuth   = position["azimuth"].get<float>();
					result_position.Elevation = position["elevation"].get<float>();
					result_position.Latitude  = position["satlatitude"].get<float>();
					result_position.Longitude = position["satlongitude"].get<float>();
				}

				return result;
			});
		}

		// @throw AL::Exception
		SatelliteRadioPassQueryResult GetRadioPasses(AL::uint32 id, AL::Float latitude, AL::Float longitude, AL::Float altitude, AL::uint8 days, AL::uint16 min_elevation)
		{
			AL::String params[] =
			{
				"radiopasses",
				AL::ToString(id).GetCString(),
				AL::ToString(latitude).GetCString(),
				AL::ToString(longitude).GetCString(),
				AL::ToString(altitude).GetCString(),
				AL::ToString(days).GetCString(),
				AL::ToString(min_elevation).GetCString()
			};

			return ExecuteQuery<SatelliteRadioPassQueryResult>(params, [](nlohmann::json& json)
			{
				SatelliteRadioPassQueryResult result =
				{
					.Result =
					{
						.Passes    = SatelliteRadioPasses(json["info"]["passescount"].get<std::uint32_t>()),
						.Satellite =
						{
							.ID   = json["info"]["satid"].get<std::uint32_t>(),
							.Name = json["info"]["satname"].get<std::string>().c_str()
						}
					},
					.TransactionCount = json["info"]["transactionscount"].get<std::uint32_t>()
				};

				AL::size_t result_pass_index = 0;

				for (auto& pass : json["passes"])
				{
					auto& result_pass = result.Result.Passes[result_pass_index++];

					result_pass.Rise          = AL::Timestamp::FromSeconds(pass["startUTC"].get<std::uint32_t>());
					result_pass.Set           = AL::Timestamp::FromSeconds(pass["endUTC"].get<std::uint32_t>());
					result_pass.Azimuth.Start = pass["startAz"].get<float>();
					result_pass.Azimuth.Max   = pass["maxAz"].get<float>();
					result_pass.Azimuth.End   = pass["endAz"].get<float>();
					result_pass.Elevation     = pass["maxEl"].get<float>();
				}

				return result;
			});
		}

		// @throw AL::Exception
		SatelliteVisiblePassQueryResult GetVisualPasses(AL::uint32 id, AL::Float latitude, AL::Float longitude, AL::Float altitude, AL::uint8 days, AL::TimeSpan min_visible_time)
		{
			AL::String params[] =
			{
				"visualpasses",
				AL::ToString(id).GetCString(),
				AL::ToString(latitude).GetCString(),
				AL::ToString(longitude).GetCString(),
				AL::ToString(altitude).GetCString(),
				AL::ToString(days).GetCString(),
				AL::ToString(min_visible_time.ToSeconds()).GetCString()
			};

			return ExecuteQuery<SatelliteVisiblePassQueryResult>(params, [](nlohmann::json& json)
			{
				SatelliteVisiblePassQueryResult result =
				{
					.Result =
					{
						.Passes    = SatelliteVisiblePasses(json["info"]["passescount"].get<std::uint32_t>()),
						.Satellite =
						{
							.ID   = json["info"]["satid"].get<std::uint32_t>(),
							.Name = json["info"]["satname"].get<std::string>().c_str()
						}
					},
					.TransactionCount = json["info"]["transactionscount"].get<std::uint32_t>()
				};

				AL::size_t result_pass_index = 0;

				for (auto& pass : json["passes"])
				{
					auto& result_pass = result.Result.Passes[result_pass_index++];

					result_pass.Rise          = AL::Timestamp::FromSeconds(pass["startUTC"].get<std::uint32_t>());
					result_pass.Set           = AL::Timestamp::FromSeconds(pass["endUTC"].get<std::uint32_t>());
					result_pass.Azimuth.Start = pass["startAz"].get<float>();
					result_pass.Azimuth.Max   = pass["maxAz"].get<float>();
					result_pass.Azimuth.End   = pass["endAz"].get<float>();
					result_pass.Elevation     = pass["maxEl"].get<float>();
					result_pass.Duration      = AL::TimeSpan::FromSeconds(pass["duration"].get<std::uint32_t>());
					result_pass.Magnitude     = pass["mag"].get<std::uint32_t>();
				}

				return result;
			});
		}

	private:
		template<AL::size_t S_URI_PARAMS>
		auto BuildUri(const AL::String(&uri_params)[S_URI_PARAMS]) const
		{
			AL::StringBuilder sb;

			sb << "https://api.n2yo.com/rest/v1/satellite";

			for (AL::size_t i = 0; i < S_URI_PARAMS; ++i)
				sb << '/' << uri_params[i];

			sb << "&apiKey=" << key;

			return AL::Network::HTTP::Uri::FromString(
				sb.ToString()
			);
		}

		// @throw AL::Exception
		template<typename T_QUERY_RESULT, AL::size_t S_URI_PARAMS>
		T_QUERY_RESULT ExecuteQuery(const AL::String(&uri_params)[S_URI_PARAMS], T_QUERY_RESULT(*decode_result)(nlohmann::json& json))
		{
			nlohmann::json json;

			{
				auto uri = BuildUri(
					uri_params
				);

				try
				{
					json = nlohmann::json::parse(
						client.DownloadString(uri).GetCString()
					);
				}
				catch (AL::Exception& exception)
				{

					throw AL::Exception(
						AL::Move(exception),
						"Error downloading '%s'",
						uri.ToString().GetCString()
					);
				}
			}

			{
				auto it = json.find(
					"error"
				);

				if (it != json.end())
				{

					throw AL::Exception(
						it->get<std::string>().c_str()
					);
				}
			}

			T_QUERY_RESULT result;

			try
			{
				result = decode_result(
					json
				);
			}
			catch (AL::Exception& exception)
			{

				throw AL::Exception(
					AL::Move(exception),
					"Error decoding result"
				);
			}
			catch (const nlohmann::json::exception& exception)
			{

				throw AL::Exception(
					"Error decoding result: %s",
					exception.what()
				);
			}

			return result;
		}
	};
}
