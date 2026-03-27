#pragma once

#define NOMINMAX
#include <pxr/base/tf/diagnosticMgr.h>

namespace usd
{

class UsdLogger : public pxr::TfDiagnosticMgr::Delegate
{
public:
    void IssueError(const pxr::TfError& err) override;
    void IssueFatalError(const pxr::TfCallContext& context,
                         const std::string& msg) override;
    void IssueStatus(const pxr::TfStatus& status) override;
    void IssueWarning(const pxr::TfWarning& warning) override;
};
} // usd