//
// Copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
//

/// @file      main.cpp
/// @author    Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @date      2019-09-30
/// @brief     Main function of the pathtracer

#include <chrono> // std::chrono::duration
#include <cstring> // std::strlen

#include "argparse.h" // argparse
#include "doctest.h" // doctest::Context

#include "AssetGeometry.h"
#include "BDPTRenderer.h"
#include "Denoiser.h"
#include "ImageSaver.h"
#include "HdrBackground.h"
#include "OpenMaterial.h"
#include "PinholeCamera.h"
#include "PhysicallyBasedMaterial.h"
#include "RenderLightPoint.h"
#include "ToneMapping.h"

/// @brief Dummy switcher between @a testing mode and @a release mode.
struct DocTestEnabler final
{
    static bool m_bEnable;
};

bool DocTestEnabler::m_bEnable = false;


/// @brief Dummy switcher between @a testing mode and @a release mode.
struct LightBoxEnabler final
{
    static bool m_bEnable;
};

bool LightBoxEnabler::m_bEnable = false;


/// @brief Utility call to read scene transformation parameters @p rp into a single 4x4 matrix @p TM .
void setUpTransformation(fmat4 &TM, const fvec3 bbc, const fvec3 rotation, const fvec3 shift)
{
    fmat4 t;
    assignIdentityMatrix(t);
    constexpr float fDegToRad = M_PIf / 180.0f;

    fvec3 a;
    std::transform(rotation, rotation + 3, std::reverse_iterator<float *>(a + 3), Scalef{fDegToRad});

    fvec3 s, c;
    std::transform(a, a + 3, s, Func1D(std::sin));
    std::transform(a, a + 3, c, Func1D(std::cos));

    fmat3 Rx = {{ 1.0f, 0.0f, 0.0f},
                { 0.0f, c[0], s[0]},
                { 0.0f,-s[0], c[0]}};
    fmat3 Ry = {{ c[1], 0.0f,-s[1]},
                { 0.0f, 1.0f, 0.0f},
                { s[1], 0.0f, c[1]}};
    fmat3 Rz = {{ c[2], s[2], 0.0f},
                {-s[2], c[2], 0.0f},
                { 0.0f, 0.0f, 1.0f}};

    fmat3 Rzy, Rzyx;
    multMatrix3(Rz, Ry, Rzy);
    multMatrix3(Rzy, Rx, Rzyx);

    std::inner_product(Rzyx, Rzyx + 3, t, 0, [](int, void *) { return 0; }, std::bind(&std::copy_n<float *, int, float *>, _1, 3, _2));
    std::tie(t[0][3], t[1][3], t[2][3]) = std::make_tuple(bbc[0], bbc[1], bbc[2]);
    transformationInverse(t, TM);

    TM[0][3] -= (shift[0] - bbc[0]);
    TM[1][3] -= (shift[1] - bbc[1]);
    TM[2][3] -= (shift[2] - bbc[2]);
}


/// @brief Utility call to save a buffer @p img of pixel colors into an image file @p fileName ,
/// with a specified resolution @p iw x @p ih .
void saveImage(const char *fileName, fvec4 *img, int iw, int ih) noexcept
{
    auto image = reinterpret_cast<float *>(img);
    unsigned char *d = nullptr;

    try {
        d = new unsigned char[4*iw*ih];
        std::transform(image, image + 4*iw*ih, d, [](float f) { return static_cast<unsigned char>(255.0f * f); });

        auto fileExt = utils::path::fileextension(fileName);
        if (fileExt == "png") {
            ImageSaver::savePng(fileName, iw, ih, 4, d);
        }
        else if (fileExt == "bmp") {
            ImageSaver::saveBmp(fileName, iw, ih, 4, d);
        }
        else if (fileExt == "tga") {
            ImageSaver::saveTga(fileName, iw, ih, 4, d);
        }
        else if (fileExt == "jpg") {
            ImageSaver::saveJpg(fileName, iw, ih, 4, d, 100);
        }
        else if (fileExt == "pfm") {
            ImageSaver::savePfm(fileName, iw, ih, reinterpret_cast<const float *>(img));
        }
        else {
            throw InvalidFormatError("This image extension is not supported");
        }
    }
    catch (const std::exception &error) {
        std::cerr << "An unexpected error occured during writing of the image:\n" << error.what() << std::endl;
    }

    delete[] d;
}


/**
 * @brief Entry point of the program.
 *
 * @param [in] argc arguments from command line
 * @param [in] argv number of arguments
 *
 * @details The placement of the objects on the scene is done in the scope of
 * @a right-handed @a Cartesian coordinate system ( @b Z-axis points towards the spectator).
 * All distances are set in @a meters and angles are measured in @a degrees.
 *
 * @return Code corresponding to the status of execution.
 */
int main(int argc, const char *argv[])
{
    ImageSaver::flipVerticallyOnWrite(true);
    RendererParameters rparams;

    float fAspect = 4.0f / 3.0f;

    const char 
        *sInputFile = "",
        *sEulerRotation = "",
        *sConsoleOutputFile = "",
        *sMaterialPriority = "om",
        *sTranslation = "";

#ifdef FC_VALIDATION
    const char
        *sFCSubject = "";
#endif // !FC_VALIDATION

    // Parse command line options
    static const char *const usage[] = {
        "pathtracer [options] [[--] args]",
        "pathtracer [options]",
        nullptr,
    };

    argparse_option options[] = {
        OPT_HELP(),

        OPT_GROUP("Required arguments"),
        OPT_STRING('i', "input", &sInputFile, "path to glTF file (supported extensions: .gltf, .glb)", nullptr, 0, 0),

        OPT_GROUP("Optional arguments"),
        OPT_INTEGER('A', "lightAttenuation", &rparams.lightDistanceAttenuation, "light distance attenuation exponent (0=no attenuation, 1=linear attenuation, 2=quadratic attenuation, " BDPT_COMMAND_LINE_DEFAULT " 1)", nullptr, 0, 0),
        OPT_FLOAT(  'a', "aspect", &fAspect, "aspect ratio of the image, width-to-height ratio (" BDPT_COMMAND_LINE_DEFAULT " 4/3)", nullptr, 0, 0),
        OPT_INTEGER('B', "cameraBounces", &rparams.cameraBouncesN, "maximum number of bounces a camera ray can achieve (" BDPT_COMMAND_LINE_DEFAULT " 10)", nullptr, 0, 0),
        OPT_INTEGER('b', "lightBounces", &rparams.lightBouncesN, "maximum number of bounces a light ray can achieve (" BDPT_COMMAND_LINE_DEFAULT " 10)", nullptr, 0, 0),
        OPT_INTEGER('C', "numberOfCores", &rparams.numberOfCores, "number of CPUs to use for rendering (" BDPT_COMMAND_LINE_DEFAULT " 1)", nullptr, 0, 0),
        OPT_BOOLEAN('c', "automaticCentering", &rparams.automaticCentering, "automatic scene centering, " BDPT_COMMAND_LINE_DEFAULT " false)", nullptr, 0, 0),
        OPT_BOOLEAN('D', "denoiser", &rparams.useDenoiser, "enables simple median denoiser and writes a denoised picture to the same directory (" BDPT_COMMAND_LINE_DEFAULT " false)", nullptr, 0, 0),
        OPT_STRING( 'e', "eulerAngles", &sEulerRotation, "Euler angles (in " BDPT_BOLD("degrees") ") of rotation in a right-handed system (Z-axis pointing towards the user, " BDPT_COMMAND_LINE_DEFAULT " (0, 0, 0)). Order of angles: \n\troll (bank angle, Z-axis) \n\tyaw (bearing, Y-axis) \n\tpitch (elevation, X-axis)", nullptr, 0, 0),
#ifdef FC_VALIDATION
        OPT_STRING( 'F', "falseColor", &sFCSubject, "subject of the false-color rendering mode (" BDPT_COMMAND_LINE_DEFAULT " " BDPT_COMMAND_UNSPECIFIED "):\n\tpid=primitiveId \n\tgid=geometryId \n\tmid=materialId \n\trmp=roughnessMap \n\tmmp=metallicMap \n\tmd=meshDensity \n\tsg=surfaceGradient \n\tmn=materialName  \n\tin=invertedNormals", nullptr, 0, 0),
#endif // !FC_VALIDATION
        OPT_FLOAT(  'g', "gamma", &rparams.gamma, "gamma correction exponent (" BDPT_COMMAND_LINE_DEFAULT " 0.5)", nullptr, 0, 0),
        OPT_STRING( 'H', "hdr", &rparams.hdrFile, "path to HDR file (" BDPT_COMMAND_LINE_DEFAULT " " BDPT_COMMAND_UNSPECIFIED ")", nullptr, 0, 0),
        OPT_BOOLEAN('L', "lights", &AssetGeometry::m_bUseLights, "use light source from glTF file (" BDPT_COMMAND_LINE_DEFAULT " false)", nullptr, 0, 0),
        OPT_STRING( 'l', "cameraLens", &rparams.cameraPropertiesFile, "path to JSON file containing camera properties (" BDPT_COMMAND_LINE_DEFAULT " " BDPT_COMMAND_UNSPECIFIED ")", nullptr, 0, 0),
        OPT_STRING( 'o', "output", &rparams.outputFile, "path to output file (supported extensions: .png, .bmp, .tga, .jpg and .pfm, " BDPT_COMMAND_LINE_DEFAULT " ../render_image.png)", nullptr, 0, 0),
        OPT_STRING( 'P', "materialPriority", &sMaterialPriority, "which material type has priority if both are used on the same mesh (om=OpenMaterial, pbr=physicallyBasedRendering, " BDPT_COMMAND_LINE_DEFAULT " om)", nullptr, 0, 0),
        OPT_INTEGER('q', "maxPathLength", &rparams.maxPathLength, "maximum length of connected path, i.e. path between the camera and a light source (" BDPT_COMMAND_LINE_DEFAULT " 8)", nullptr, 0, 0),
        OPT_INTEGER('r', "resolution", &rparams.outputWidth, BDPT_BOLD("width") " resolution (" BDPT_COMMAND_LINE_DEFAULT " 800)", nullptr, 0, 0),
        OPT_INTEGER('S', "samples", &rparams.samplesPerPixel, "number of samples per each pixel (" BDPT_COMMAND_LINE_DEFAULT " 20)", nullptr, 0, 0),
        OPT_STRING( 'T', "translation", &sTranslation, "XYZ translation of the glTF model (" BDPT_COMMAND_LINE_DEFAULT " (0, 0, 0))", nullptr, 0, 0),
        OPT_BOOLEAN('t', "test", &DocTestEnabler::m_bEnable, "run unit tests and exit", nullptr, 0, 0),
        OPT_BOOLEAN('V', "lightBox", &LightBoxEnabler::m_bEnable, "enables light box validation (" BDPT_COMMAND_LINE_DEFAULT " false)", nullptr, 0, 0),
        OPT_STRING( 'w', "writeToFile", &sConsoleOutputFile, "reroute console output to a provided text file (" BDPT_COMMAND_LINE_DEFAULT " " BDPT_COMMAND_UNSPECIFIED ")", nullptr, 0, 0),
        OPT_END(),
    };

    argparse argparse;
    argparse_init(&argparse, options, usage, 0);
    argparse_describe(&argparse, "\nRender a glTF object with or without HDR background", nullptr);
    argc = argparse_parse(&argparse, argc, argv);

    rparams.outputHeight = static_cast<int>(rparams.outputWidth / fAspect); // conversion int to float is ok

    AssetGeometry::m_sMaterialLoadingPriority = sMaterialPriority;
    sMaterialPriority = nullptr;

    // Run unit tests
    if (DocTestEnabler::m_bEnable) {
        doctest::Context ctx;
        // Stop after 5 failed asserts
        ctx.setOption("abort-after", 5);
        ctx.run();
        return 0;
    }

    // Write std::cout output to txt file
    if (std::strlen(sConsoleOutputFile)) {
        auto f = freopen(sConsoleOutputFile, "w", stdout);
        BDPT_UNUSED(f);
    }

    if (!std::strlen(sInputFile)) {
        std::cerr << "Missing parameter --input" << std::endl << std::endl;
        argparse_usage(&argparse);
        return 1;
    }

    if (!utils::path::fileExists(sInputFile)) {
        std::cerr << "File " << sInputFile << " does not exist" << std::endl;
        return 1;
    }

    if (AssetGeometry::m_sMaterialLoadingPriority != "om" && AssetGeometry::m_sMaterialLoadingPriority != "pbr") {
        std::cerr << "Material loading priority has to be 'om' or 'pbr'" << std::endl;
        return 1;
    }

    if (std::strlen(sEulerRotation)) {
        auto junks = utils::string::split(sEulerRotation, ",");
        try {
            std::transform(junks.cbegin(), std::next(junks.cbegin(), 3), rparams.sceneRotationZYXdeg, std::bind(static_cast<float(*)(const std::string &, size_t *)>(std::stof), _1, nullptr));
            sEulerRotation = nullptr;
        }
        catch (...) {
            std::cerr << "Invalid format for Euler angles" << std::endl;
            return 1;
        }
    }

    if (std::strlen(sTranslation)) {
        auto junks = utils::string::split(sTranslation, ",");
        try {
            std::transform(junks.cbegin(), std::next(junks.cbegin(), 3), rparams.sceneTranslation, std::bind(static_cast<float(*)(const std::string &, size_t *)>(std::stof), _1, nullptr));
            std::transform(rparams.sceneTranslation, rparams.sceneTranslation + 3, rparams.sceneTranslation, std::negate<float>{});
            sTranslation = nullptr;
        }
        catch (...) {
            std::cerr << "Invalid format for translation" << std::endl;
            return 1;
        }
    }

#ifdef FC_VALIDATION
    if (std::strlen(sFCSubject)) {
        rparams.falseColorMode = true;
        rparams.cameraBouncesN = rparams.lightBouncesN = 1;
        rparams.hdrFile = "";
        rparams.samplesPerPixel = 1;
        rparams.useDenoiser = false;
        AssetGeometry::m_bUseLights = false;
    }
#endif // !FC_VALIDATION

    // Load glTF file
    std::cout << "Loading " << sInputFile << std::endl;
    AssetGeometry assetGeometry(sInputFile);
    std::cout << "Loaded " << assetGeometry.getTitle() << std::endl;

#ifdef MATERIAL_VERBOSE_DUMP
    std::cout << std::endl << "Loaded materials (" << assetGeometry.m_vpMaterials.size() << "):" << std::endl;
    for (const auto &mat : assetGeometry.m_vpMaterials) {
        std::cout << "--------------------------------\n" << *mat;
    }
    std::cout << "--------------------------------\n" << std::endl;
#endif // !MATERIAL_VERBOSE_DUMP

    auto BB = assetGeometry.getBBox();
    fvec3 BBC;
    BB.getCenter(BBC);

    // if (rparams.automaticCentering) {
    //     assignVector3(rparams.sceneTranslation, BBC);
    //     std::cout << "Automatic centering is enabled" << std::endl;
    // }

    std::cout << "Building scene" << std::endl;

    fmat4 transformation;
    LightBoxEnabler::m_bEnable ? assignIdentityMatrix(transformation) : setUpTransformation(transformation, BBC, rparams.sceneRotationZYXdeg, rparams.sceneTranslation);

    RenderScene rsc;
    int instN = 0;
    iterateAssetGeometryNodes(&assetGeometry, transformation, [&](Node* n, const fmat4) {
        instN += static_cast<int>(n->m_vpPrimitives.size());
    });

    std::map<const Material*, int> mats;
    int matId = 0;
    for (Mesh* m : assetGeometry.m_vpPrimitives) {
        if (m->m_pMaterial && m->m_pMaterial != assetGeometry.m_pMissingMaterial) {
            auto sr = mats.find(m->m_pMaterial);
            if (sr == mats.end()) mats[m->m_pMaterial] = matId++;
        }
    }

    // Allocating memory for scene structures
    rsc.allocate(static_cast<int>(assetGeometry.m_vpPrimitives.size()), instN,
                 static_cast<int>(mats.size()),
                 static_cast<int>(assetGeometry.m_vImages.size()),
                 static_cast<int>(assetGeometry.m_vpLights.size()));

    // Setting textures
    for (auto &t : assetGeometry.m_vImages) {
        rsc.setTexture(t.m_iId, t.m_iWidth, t.m_iHeight, t.m_pImage);
    }

    // Setting materials
    for (auto &m : mats) {
        // Here a class inheriting from RenderMaterial should be created based on m.first!
        union {
            const AssetMaterial *om;
            const PbrMaterial *pbr;
        } genMat {nullptr};

        RenderMaterial *mat = nullptr;

        if ((genMat.om = dynamic_cast<const AssetMaterial*>(m.first))) {
            auto om = new OpenMaterial;
            om->setMaterial(genMat.om);
            mat = om;
        }
        else if ((genMat.pbr = dynamic_cast<const PbrMaterial*>(m.first))) {
            auto pbr = new PhysicallyBasedMaterial;
            pbr->setMaterial(genMat.pbr);
            mat = pbr;
        }
        else {
            std::cerr << "Invalid material found" << std::endl;
            return 1;
        }

        rsc.setMaterial(m.second, mat); // rsc owns material and deletes it
    }

    // Setting lights
    int id = 0;
    for (auto &l : assetGeometry.m_vpLights) {

        fmat4 gNT, tM;
        l->getNode()->getGlobalTransformation(gNT);
        multSquareMatrix(transformation, gNT, tM);
        const fvec3 lPos = {tM[0][3], tM[1][3], tM[2][3]};

        fvec3 lInt = {l->getLightRed(), l->getLightGreen(), l->getLightBlue()};
        std::transform(lInt, lInt + 3, lInt, Scalef{l->getLightIntensity()});

        auto lgt = new RenderLightPoint;
        lgt->set(lPos, lInt, l->getLightRange());
        rsc.setLight(id++, lgt); // rsc owns light and deletes it
    }

    // Setting meshes
    id = 0;
    for (auto &m : assetGeometry.m_vpPrimitives) {
        float* tc[MAX_UV_CHANNELS];
        int maxTn = 0;
        for (int i = 0; i < MAX_UV_CHANNELS; ++i) {
            tc[i] = nullptr;
            if (m->m_vfTexCoordBuffers[i].empty()) continue;
            maxTn = i+1;
            tc[i] = m->m_vfTexCoordBuffers[i].data();
        }
        matId = (m->m_pMaterial && m->m_pMaterial != assetGeometry.m_pMissingMaterial) ? mats[m->m_pMaterial] : -1;
        rsc.setMesh(id++,
            static_cast<int>(m->m_vfVertexBuffer.size())/3,
            static_cast<int>(m->m_vuiIndexBuffer.size())/3,
            maxTn,matId,m->m_vuiIndexBuffer.data(),m->m_vfVertexBuffer.data(),3,
            m->m_vfNormalBuffer.data(),3,m->m_vfTangentBuffer.data(),3,tc,2
        );
    }

    // Setting instances
    instN = 0;
    iterateAssetGeometryNodes(&assetGeometry, transformation, [&](Node* n, const fmat4 tM) {
        for (auto &m : n->m_vpPrimitives)
            rsc.setInstance(instN++, tM, m->m_iId);
    });

    // Setting hdr background
    if (!AssetGeometry::m_bUseLights && utils::path::fileExists(rparams.hdrFile)) {
        auto b = new HDRBackground(rparams.hdrFile);
        // Since gltfs have y up and backgrounds have z up, we rotate background to y up
        const fmat3 R = {{1.0f, 0.0f, 0.0f},
                         {0.0f, 0.0f, 1.0f},
                         {0.0f,-1.0f, 0.0f}};
        b->setRotation(R);
        b->setRadianceScale(rparams.hdrScaleValue);
        rsc.setBackground(b); // rsc owns background and deletes it
    }

#ifdef FC_VALIDATION
    if (rparams.falseColorMode) {
        rsc.setFalseColorSubject(sFCSubject);
    }
#endif // !FC_VALIDATION

    std::list<std::string> log;
    bool sres = rsc.commitScene(&log);
    std::copy(log.begin(), log.end(), std::ostream_iterator<std::string>(std::cout, "\n"));

    if (!sres) {
        std::cerr << "Incomplete scene cannot be rendered!" << std::endl;
        return 1;
    }

    std::cout << "Built" << std::endl << std::endl;

    BDPTRenderer renderer;
    renderer.setParameters(&rparams);
    renderer.setScene(&rsc);

    std::vector<ViewPoint> viewPoints;
    viewPoints.reserve(1);

    // Set up all viewpoints
    if (LightBoxEnabler::m_bEnable) {
        // Light box scenario
        viewPoints.reserve(8);

        fvec3 C;
        float P[8][3];
        BB.getCenter(C);
        BB.getCorners(P);

        std::transform(P, P + 8, std::back_inserter(viewPoints), [C](float *p) -> ViewPoint {
            fvec3 vViewUp = {0.0f, 1.0f, 0.0f};

            fvec3 vViewForward;
            std::transform(C, C + 3, p, vViewForward, std::minus<float>{});

            fvec3 vViewPosition;
            std::transform(vViewForward, vViewForward + 3, C, vViewPosition, std::bind(Func3D(linearFunction), _1, _2, -2.0f));
            normalize3(vViewForward);

            fvec3 vViewLeft;
            cross3(vViewForward, vViewUp, vViewLeft);
            cross3(vViewLeft, vViewForward, vViewUp);
            return {vViewPosition, vViewLeft, vViewUp, vViewForward};
        });
    }
    else {
        // Default setting for camera
        fvec3 vViewUp       = {0.00f, 1.00f, 0.00f};
        fvec3 vViewPosition = {0.00f, 0.00f, 0.00f};
        fvec3 vViewForward  = {0.00f, 0.00f,-1.00f};
        if (rparams.automaticCentering) {
            fvec3 TBBC;
            multVector3(transformation, BBC, TBBC);
            std::transform(TBBC, TBBC + 3, vViewPosition, vViewForward, std::minus<float>{});
            if (length3(vViewForward) < fEpsilon) assignVector(vViewForward, 0.00f, 0.00f,-1.00f);
            normalize3(vViewForward);
            std::cout << "Automatic centering is enabled" << std::endl;
        }

        fvec3 vViewLeft;
        cross3(vViewForward, vViewUp, vViewLeft);
        cross3(vViewLeft, vViewForward, vViewUp);
        viewPoints.emplace_back(vViewPosition, vViewLeft, vViewUp, vViewForward);
    }

    // Setting up the camera
    PinholeCamera pcam(rparams.outputWidth, rparams.outputHeight);
    if (!LightBoxEnabler::m_bEnable && std::strlen(rparams.cameraPropertiesFile)) {
        pcam.loadProperties(rparams.cameraPropertiesFile);
    }
    else {
        pcam.adjustToResolution();
        pcam.setYFoV(60.0f);
    }

    // Loop over all view points of the camera
    int vpi = 0;
    std::for_each(viewPoints.begin(), viewPoints.end(), [&rparams, &pcam, &renderer, &vpi](const ViewPoint &vp) {
        pcam.setViewPoint(vp);
        std::string outputFile = rparams.outputFile;
        if (LightBoxEnabler::m_bEnable) {
            outputFile.insert(outputFile.find_last_of('.'), '_' + std::to_string(vpi++));
        }

        std::cout << "Rendering scene" << std::endl;
        auto startTime = std::chrono::high_resolution_clock::now();
        renderer.render(&pcam);
        auto endTime = std::chrono::high_resolution_clock::now();
        std::cout << "Rendered" << std::endl;

        fvec4* img = new fvec4[rparams.outputWidth * rparams.outputHeight];
        
        std::chrono::duration<float, std::milli> duration = endTime - startTime;
        std::cout << std::endl << "Duration: " << duration.count() / 1000.0f << " seconds" << std::endl; 

        for (int i = 0; i < rparams.outputHeight; ++i)
            for (int j = 0; j < rparams.outputWidth; ++j)
                pcam.getImpression(j, i, img[i*rparams.outputWidth + j]);

        gammaCorrection(img, rparams.outputWidth, rparams.outputHeight, 1.0f, rparams.gamma);

        std::cout << "Saving image to " << outputFile << std::endl;
        saveImage(outputFile.c_str(), img, rparams.outputWidth, rparams.outputHeight);
        if (rparams.useDenoiser) {
            SimpleDenoiserMedian<1>(img, rparams.outputWidth, rparams.outputHeight);
            std::string outputFileDenoised = std::move(outputFile);
            saveImage(outputFileDenoised.insert(outputFileDenoised.find_last_of('.'), "_denoised").c_str(), img, rparams.outputWidth, rparams.outputHeight);
        }
        std::cout << "Saved" << std::endl;

        delete[] img;
    });

    return 0;
}
