#include "usdprocessor.h"

#include <filesystem>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <pxr/base/tf/diagnosticMgr.h>
#include <pxr/usd/sdf/layer.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/base/plug/registry.h>
#include <pxr/usd/usd/primRange.h>

#include <logger.h>
#include <tooling/platformutils.h>

class USDLogger : public pxr::TfDiagnosticMgr::Delegate
{
public:
	void IssueError(const pxr::TfError &err) override
	{
		Logger::error(this, std::format("{}", err.GetErrorCodeAsString()));
	}
	void IssueFatalError(const pxr::TfCallContext &context,
		const std::string &msg) override
	{
		Logger::error(this, std::format("USD Fatal Error: {}", msg));
	}
	void IssueStatus(const pxr::TfStatus &status) override
	{
		Logger::info(this, std::format("USD Status: {}", status.GetCommentary()));
	}
	void IssueWarning(const pxr::TfWarning &warning) override
	{
		Logger::warn(this, std::format("{}", warning.GetCommentary()));
	}
};

void USDProcessor::loadStage()
{
	using namespace pxr;

	// Activate debug symbols programmatically (alternative to env var)
	USDLogger usdLogger;
	TfDiagnosticMgr::GetInstance().AddDelegate(&usdLogger);
	PlugRegistry &plugReg = pxr::PlugRegistry::GetInstance();

	const std::string usdPath = "S:\\projects\\constref\\sdl3-demo\\data\\usd\\ufo.usd";
	if (!std::filesystem::exists(usdPath))
	{
		throw std::runtime_error("Unable to find USD file");
	}
	auto stage = pxr::UsdStage::Open(usdPath);
	auto range = stage->Traverse();
	for (auto itr = range.begin(); itr != range.end(); ++itr)
	{
		UsdPrim prim = *itr;
		if (prim.GetTypeName() == UsdGeomTokens->Mesh)
		{
			Logger::info(this, "Mesh found, parsing");
			UsdAttribute points = prim.GetAttribute(UsdGeomTokens->Points);
		}
	}
}
