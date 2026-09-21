// Regression for issue #1320: vc_obj2tifxyz must not report success when
// rasterization produces zero valid grid points. This also drives fork CI.
//
// The synthetic OBJ has normalized [0,1] UV bounds but four small triangles
// that miss every sample of the default 2x2 grid. With the default
// stretch_factor=1 it therefore rasterizes zero points. Increasing the
// stretch factor makes the exact same mesh usable.

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <random>
#include <sstream>
#include <string>

namespace fs = std::filesystem;

namespace {

fs::path findBinary()
{
    if (const char* env = std::getenv("VC_OBJ2TIFXYZ_BIN")) {
        fs::path p(env);
        if (fs::is_regular_file(p)) return p;
    }
    return {};
}

std::string quote(const fs::path& p)
{
    return "\"" + p.string() + "\"";
}

std::string readText(const fs::path& p)
{
    std::ifstream f(p);
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

void writeNormalizedUvObj(const fs::path& path)
{
    std::ofstream f(path);

    // Four small triangles touch the min/max U/V bounds independently but
    // none contains a corner of the global [0,1]x[0,1] UV square.
    const float uv[12][2] = {
        {0.00f, 0.45f}, {0.10f, 0.40f}, {0.10f, 0.50f}, // left
        {1.00f, 0.45f}, {0.90f, 0.40f}, {0.90f, 0.50f}, // right
        {0.45f, 0.00f}, {0.40f, 0.10f}, {0.50f, 0.10f}, // bottom
        {0.45f, 1.00f}, {0.40f, 0.90f}, {0.50f, 0.90f}, // top
    };

    for (int i = 0; i < 12; ++i) {
        f << "v " << uv[i][0] * 100.0f << " " << uv[i][1] * 100.0f
          << " " << 10.0f + i * 0.01f << "\n";
    }
    for (int i = 0; i < 12; ++i) {
        f << "vt " << uv[i][0] << " " << uv[i][1] << "\n";
    }
    for (int tri = 0; tri < 4; ++tri) {
        const int a = tri * 3 + 1;
        f << "f " << a << "/" << a << " "
          << a + 1 << "/" << a + 1 << " "
          << a + 2 << "/" << a + 2 << "\n";
    }
}

} // namespace

TEST_CASE("vc_obj2tifxyz rejects zero-valid output instead of reporting success")
{
    const fs::path bin = findBinary();
    REQUIRE_MESSAGE(!bin.empty(),
                    "vc_obj2tifxyz binary not found; set VC_OBJ2TIFXYZ_BIN");

    std::random_device rd;
    std::mt19937_64 rng(rd());
    const fs::path root =
        fs::temp_directory_path() / ("vc_obj2tifxyz_zero_" + std::to_string(rng()));
    fs::create_directories(root);

    const fs::path obj = root / "normalized.obj";
    const fs::path emptyOut = root / "empty";
    const fs::path emptyLog = root / "empty.log";
    writeNormalizedUvObj(obj);

    const std::string defaultCmd =
        quote(bin) + " " + quote(obj) + " " + quote(emptyOut)
        + " > " + quote(emptyLog) + " 2>&1";
    const int defaultRc = std::system(defaultCmd.c_str());

    INFO("default-run log: ", emptyLog.string());
    CHECK(defaultRc != 0);
    CHECK_FALSE(fs::exists(emptyOut / "x.tif"));
    CHECK_FALSE(fs::exists(emptyOut / "y.tif"));
    CHECK_FALSE(fs::exists(emptyOut / "z.tif"));

    const std::string log = readText(emptyLog);
    CHECK(log.find("Valid grid points: 0 / 4") != std::string::npos);
    CHECK(log.find("refusing to save an empty tifxyz") != std::string::npos);
    CHECK(log.find("stretch_factor") != std::string::npos);
    CHECK(log.find("Successfully converted to tifxyz format") == std::string::npos);

    // The safety check must not reject the mesh itself: increasing sampling
    // density should rasterize it and produce a normal tifxyz.
    const fs::path goodOut = root / "good";
    const fs::path goodLog = root / "good.log";
    const std::string sampledCmd =
        quote(bin) + " " + quote(obj) + " " + quote(goodOut)
        + " 20 1.0 > " + quote(goodLog) + " 2>&1";
    const int sampledRc = std::system(sampledCmd.c_str());

    INFO("sampled-run log: ", goodLog.string());
    REQUIRE_MESSAGE(sampledRc == 0,
                    "explicit stretch_factor conversion failed; see " << goodLog.string());
    CHECK(fs::is_regular_file(goodOut / "x.tif"));
    CHECK(fs::is_regular_file(goodOut / "y.tif"));
    CHECK(fs::is_regular_file(goodOut / "z.tif"));

    std::error_code ec;
    fs::remove_all(root, ec);
}
