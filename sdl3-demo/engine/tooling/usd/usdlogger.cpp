#include "usdlogger.h"
#include <logger.h>

namespace usd
{
void UsdLogger::IssueError(const pxr::TfError& err)
{
    Logger::error(this, std::format("{}", err.GetErrorCodeAsString()));
}

void UsdLogger::IssueFatalError(const pxr::TfCallContext& context, const std::string& msg)
{
    Logger::error(this, std::format("USD Fatal Error: {}", msg));
}

void UsdLogger::IssueStatus(const pxr::TfStatus& status)
{
    Logger::info(this, std::format("USD Status: {}", status.GetCommentary()));
}

void UsdLogger::IssueWarning(const pxr::TfWarning& warning)
{
    Logger::warn(this, std::format("{}", warning.GetCommentary()));
}
} // usd