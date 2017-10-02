/*
 Copyright (C) 2011-2012 Gabor Papp

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as published
 by the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public License
 along with this program. If not, see <http://www.gnu.org/licenses/>.

 Based on ofxAssimpModelLoader by Anton Marini, Memo Akten, Kyle McDonald
 and Arturo Castro
*/

#include <assert.h>

#include "../assimp/include/assimp/types.h"
#include "../assimp/include/assimp/Importer.hpp"
#include "../assimp/include/assimp/scene.h"
#include "../assimp/include/assimp/postprocess.h"

#include "../assimp/code/ProcessHelper.h"

#include "../assimp/include/assimp/Logger.hpp"
#include "../assimp/include/assimp/DefaultLogger.hpp"

#include "../../Cinder-VNM/include/AssetManager.h"

#include "cinder/app/App.h"
#include "cinder/CinderMath.h"
#include "cinder/Utilities.h"
#include "cinder/TriMesh.h"
#include "cinder/Log.h"

#include "cinder/gl/draw.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/scoped.h"

#include "AssimpScene.h"

using namespace std;

inline vec3 fromAssimp(const aiVector3D &v)
{
    return vec3(v.x, v.y, v.z);
}

inline aiVector3D toAssimp(const vec3 &v)
{
    return aiVector3D(v.x, v.y, v.z);
}

inline quat fromAssimp(const aiQuaternion &q)
{
    return quat(q.w, q.x, q.y, q.z);
}

inline mat4 fromAssimp(const aiMatrix4x4 &m)
{
    return glm::make_mat4(&m.a1);
}

inline aiMatrix4x4 toAssimp(const mat4 &m)
{
    return aiMatrix4x4(
        m[0][0], m[0][1], m[0][2], m[0][3],
        m[1][0], m[1][1], m[1][2], m[1][3],
        m[2][0], m[2][1], m[2][2], m[2][3],
        m[3][0], m[3][1], m[3][2], m[3][3]);
}

inline aiQuaternion toAssimp(const quat &q)
{
    return aiQuaternion(q.w, q.x, q.y, q.z);
}

inline ColorAf fromAssimp(const aiColor4D &c)
{
    return ColorAf(c.r, c.g, c.b, c.a);
}

inline aiColor4D toAssimp(const ColorAf &c)
{
    return aiColor4D(c.r, c.g, c.b, c.a);
}

inline std::string fromAssimp(const aiString &s)
{
    return std::string(s.C_Str());
}

namespace assimp
{
    struct Logger : public ::Assimp::Logger
    {
        void OnDebug(const char* message) {
            CI_LOG_D(message);
        }

        void OnInfo(const char* message) {
            CI_LOG_I(message);
        }

        void OnWarn(const char* message) {
            CI_LOG_W(message);
        }

        void OnError(const char* message) {
            CI_LOG_E(message);
        }

        bool attachStream(::Assimp::LogStream *pStream, unsigned int severity) {
            (void)pStream; (void)severity; //this avoids compiler warnings
            return true;
        }

        bool detatchStream(::Assimp::LogStream *pStream, unsigned int severity) {
            (void)pStream; (void)severity; //this avoids compiler warnings
            return true;
        }

    };
    Logger ciLogger;

    void setupLogger()
    {
        ::Assimp::DefaultLogger::set(&ciLogger);
    }

    const int kTextureTypeCount = aiTextureType_UNKNOWN + 1;

    enum BlendMode
    {
        BlendOff,
        BlendDefault,
        BlendAdditive,
    };

    struct Mesh
    {
        const aiMesh *mAiMesh;

        gl::TextureRef mTextures[kTextureTypeCount];

        std::vector< uint32_t > mIndices;

        Material mMaterial;
        bool mTwoSided;

        BlendMode blendMode = BlendOff;

        std::vector< aiVector3D > mAnimatedPos;
        std::vector< aiVector3D > mAnimatedNorm;

        std::string mName;
        TriMeshRef mCachedTriMesh;
        TriMesh::Format meshFormat;

        bool mValidCache;

        void setupFromAiMesh(const aiMesh *aim);
        void draw();
    };

    void Mesh::setupFromAiMesh(const aiMesh *aim)
    {
        mAiMesh = aim;

        {
            if (aim->HasPositions())                meshFormat.positions();
            if (aim->HasNormals())                  meshFormat.normals();
            if (aim->HasTangentsAndBitangents())    meshFormat.tangents();
            if (aim->GetNumColorChannels() > 0)     meshFormat.colors(4);
            if (aim->GetNumUVChannels() > 0)        meshFormat.texCoords0();
        }

        mCachedTriMesh = TriMesh::create(meshFormat);

        // copy vertices
        for (unsigned i = 0; i < aim->mNumVertices; ++i)
        {
            mCachedTriMesh->appendPosition(fromAssimp(aim->mVertices[i]));
        }

        if (aim->HasNormals())
        {
            for (unsigned i = 0; i < aim->mNumVertices; ++i)
            {
                mCachedTriMesh->appendNormal(fromAssimp(aim->mNormals[i]));
            }
        }

        if (aim->HasTangentsAndBitangents())
        {
            for (unsigned i = 0; i < aim->mNumVertices; ++i)
            {
                mCachedTriMesh->appendTangent(fromAssimp(aim->mTangents[i]));
            }
        }

        // aiVector3D * mTextureCoords [AI_MAX_NUMBER_OF_TEXTURECOORDS]
        // just one for now
        if (aim->GetNumUVChannels() > 0)
        {
            for (unsigned i = 0; i < aim->mNumVertices; ++i)
            {
                mCachedTriMesh->appendTexCoord({ aim->mTextureCoords[0][i].x, aim->mTextureCoords[0][i].y });
            }
        }

        //aiColor4D *mColors [AI_MAX_NUMBER_OF_COLOR_SETS]
        if (aim->GetNumColorChannels() > 0)
        {
            for (unsigned i = 0; i < aim->mNumVertices; ++i)
            {
                mCachedTriMesh->appendColorRgba(fromAssimp(aim->mColors[0][i]));
            }
        }

        for (unsigned i = 0; i < aim->mNumFaces; ++i)
        {
            if (aim->mFaces[i].mNumIndices > 3)
            {
                throw ci::Exception("non-triangular face found: model " +
                    string(aim->mName.data) + ", face #" +
                    toString< unsigned >(i));
            }

            mCachedTriMesh->appendTriangle(aim->mFaces[i].mIndices[0],
                aim->mFaces[i].mIndices[1],
                aim->mFaces[i].mIndices[2]);
        }
    }

    Scene::Scene() : mSkinningEnabled(false),
        mAnimationEnabled(false),
        mAnimationIndex(0)
    {

    }

    shared_ptr<nodes::Node3D> Scene::create(fs::path filename)
    {
        shared_ptr<Node3D> newScene = make_shared<Scene>();
        Scene* scenePtr = (Scene*)newScene.get();

        unsigned flags = 
            aiProcess_Triangulate |
            //aiProcess_FlipUVs |
            aiProcessPreset_TargetRealtime_Quality |
            aiProcess_FindInstances |
            //aiProcess_ValidateDataStructure |
            aiProcess_OptimizeMeshes
            ;

        scenePtr->mAiImporter = make_shared<Assimp::Importer>();
        scenePtr->mAiImporter->SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT);
        scenePtr->mAiImporter->SetPropertyInteger(AI_CONFIG_PP_PTV_NORMALIZE, true);

        scenePtr->mAiScene = scenePtr->mAiImporter->ReadFile(filename.string(), flags);
        if (!scenePtr->mAiScene)
            throw ci::Exception(scenePtr->mAiImporter->GetErrorString());

        scenePtr->mFilePath = filename;
        //scenePtr->setName(filename.filename().string());

        scenePtr->setupSceneMeshes();
        scenePtr->setupNodes(scenePtr->mAiScene->mRootNode, newScene);

        return newScene;
    }
    
    AxisAlignedBox Scene::getBoundingBox()
    {
        static bool isDirty = true;
        if (isDirty)
        {
            isDirty = false;
            vec3 aMin, aMax;
            calculateBoundingBox(&aMin, &aMax);
            mBoundingBox = AxisAlignedBox(aMin, aMax);
        }
        return mBoundingBox;
    }

    void Scene::calculateBoundingBox(vec3 *min, vec3 *max)
    {
        aiMatrix4x4 trafo;

        aiVector3D aiMin, aiMax;
        aiMin.x = aiMin.y = aiMin.z = 1e10f;
        aiMax.x = aiMax.y = aiMax.z = -1e10f;

        calculateBoundingBoxForNode(mAiScene->mRootNode, &aiMin, &aiMax, &trafo);
        *min = fromAssimp(aiMin);
        *max = fromAssimp(aiMax);
    }

    void Scene::calculateBoundingBoxForNode(const aiNode *nd, aiVector3D *min, aiVector3D *max, aiMatrix4x4 *trafo)
    {
        aiMatrix4x4 prev;

        prev = *trafo;
        *trafo = *trafo * nd->mTransformation;

        for (unsigned n = 0; n < nd->mNumMeshes; ++n)
        {
            const aiMesh *mesh = mAiScene->mMeshes[nd->mMeshes[n]];
            for (unsigned t = 0; t < mesh->mNumVertices; ++t)
            {
                aiVector3D tmp = mesh->mVertices[t];
                tmp *= (*trafo);

                min->x = math<float>::min(min->x, tmp.x);
                min->y = math<float>::min(min->y, tmp.y);
                min->z = math<float>::min(min->z, tmp.z);
                max->x = math<float>::max(max->x, tmp.x);
                max->y = math<float>::max(max->y, tmp.y);
                max->z = math<float>::max(max->z, tmp.z);
            }
        }

        for (unsigned n = 0; n < nd->mNumChildren; ++n)
        {
            calculateBoundingBoxForNode(nd->mChildren[n], min, max, trafo);
        }

        *trafo = prev;
    }

    void Scene::setupNodes(const aiNode *nd, nodes::Node3DRef parentRef)
    {
        MeshNodeRef nodeRef = make_shared<MeshNode>();
        if (parentRef) parentRef->addChild(nodeRef);

        string nodeName = fromAssimp(nd->mName);
        nodeRef->setName(nodeName);
        mNodeMap[nodeName] = nodeRef;

        // store transform
        aiVector3D scaling;
        aiQuaternion rotation;
        aiVector3D position;
        nd->mTransformation.Decompose(scaling, rotation, position);
        nodeRef->setScale(fromAssimp(scaling));
        nodeRef->setRotation(fromAssimp(rotation));
        nodeRef->setPosition(fromAssimp(position));

        // meshes
        for (auto i = 0; i < nd->mNumMeshes; ++i)
        {
            auto meshId = nd->mMeshes[i];
            //if (meshId >= mMeshes.size())
            //    throw ci::Exception("node " + nodeRef->getName() + " references mesh #" +
            //        toString< unsigned >(meshId) + " from " +
            //        toString< size_t >(mMeshes.size()) + " meshes.");
            nodeRef->mMeshes.push_back(mAllMeshes[meshId]);
        }

        // store the node with meshes for rendering
        if (nd->mNumMeshes > 0)
        {
            mAllNodes.push_back(nodeRef);
        }

        // process all children
        for (unsigned n = 0; n < nd->mNumChildren; ++n)
        {
            setupNodes(nd->mChildren[n], nodeRef);
        }
    }

    MeshRef Scene::convertAiMesh(const aiMesh *mesh)
    {
        // the current AssimpMesh we will be populating data into.
        MeshRef meshRef(new Mesh());

        meshRef->mName = fromAssimp(mesh->mName);

        // Handle material info
        aiMaterial *mtl = mAiScene->mMaterials[mesh->mMaterialIndex];

        aiString name;
        mtl->Get(AI_MATKEY_NAME, name);
        CI_LOG_I("material " << fromAssimp(name));

        // Culling
        int twoSided;
        if ((AI_SUCCESS == mtl->Get(AI_MATKEY_TWOSIDED, twoSided)) && twoSided)
        {
            meshRef->mTwoSided = true;
            meshRef->mMaterial.Face = GL_FRONT_AND_BACK;
            CI_LOG_I(" two sided");
        }
        else
        {
            meshRef->mTwoSided = false;
            meshRef->mMaterial.Face = GL_FRONT;
        }

        aiColor4D dcolor, scolor, acolor, ecolor;
        if (AI_SUCCESS == mtl->Get(AI_MATKEY_COLOR_DIFFUSE, dcolor))
        {
            meshRef->mMaterial.Diffuse = fromAssimp(dcolor);
            CI_LOG_I(" diffuse: " << fromAssimp(dcolor));
        }

        if (AI_SUCCESS == mtl->Get(AI_MATKEY_COLOR_SPECULAR, scolor))
        {
            meshRef->mMaterial.Specular = fromAssimp(scolor);
            CI_LOG_I(" specular: " << fromAssimp(scolor));
        }

        if (AI_SUCCESS == mtl->Get(AI_MATKEY_COLOR_AMBIENT, acolor))
        {
            meshRef->mMaterial.Ambient = fromAssimp(acolor);
            CI_LOG_I(" ambient: " << fromAssimp(acolor));
        }

        if (AI_SUCCESS == mtl->Get(AI_MATKEY_COLOR_EMISSIVE, ecolor))
        {
            meshRef->mMaterial.Emission = fromAssimp(ecolor);
            CI_LOG_I(" emission: " << fromAssimp(ecolor));
        }

        // FIXME: not sensible data, obj .mtl Ns 96.078431 -> 384.314
        float shininessStrength = 1;
        if ( AI_SUCCESS == mtl->Get( AI_MATKEY_SHININESS_STRENGTH, shininessStrength ) )
        {
            CI_LOG_I("shininess strength: " << shininessStrength);
        }
        float shininess;
        if ( AI_SUCCESS == mtl->Get( AI_MATKEY_SHININESS, shininess ) )
        {
            meshRef->mMaterial.Shininess = shininess * shininessStrength;
            CI_LOG_I("shininess: " << shininess * shininessStrength << "[" <<
                shininess << "]");
        }

        int blendMode;
        if (AI_SUCCESS == aiGetMaterialInteger(mtl, AI_MATKEY_BLEND_FUNC, &blendMode)) {
            if (blendMode == aiBlendMode_Default) meshRef->blendMode = BlendDefault;
            else if (blendMode == aiBlendMode_Additive) meshRef->blendMode = BlendAdditive;
        }

        // Load Textures
        const int kTexSlot = 0; // TODO:
        for (int type = 0; type < kTextureTypeCount; type++)
        {
            auto aiType = (aiTextureType)type;
            aiString texPath;
            if (AI_SUCCESS == mtl->GetTexture(aiType, kTexSlot, &texPath))
            {
                CI_LOG_I(Assimp::TextureTypeToString(aiType) << " - " << texPath.data);
                string temp(texPath.data);
                replace(temp.begin(), temp.end(), '\\', '/');
                fs::path texFsPath(temp);
                fs::path modelFolder = mFilePath.parent_path();
                fs::path relTexPath = texFsPath.parent_path();
                fs::path texFile = texFsPath.filename();
                fs::path realPath = modelFolder / relTexPath / texFile;
                CI_LOG_I(realPath.string());

                gl::Texture::Format format;
                format.enableMipmapping();
                format.setMinFilter(GL_LINEAR_MIPMAP_LINEAR);
                format.setMagFilter(GL_LINEAR);
                format.setWrap(GL_REPEAT, GL_REPEAT);
                
                int uwrap;
                if (AI_SUCCESS == mtl->Get(AI_MATKEY_MAPPINGMODE_U_DIFFUSE(0), uwrap))
                {
                    switch (uwrap)
                    {
                    case aiTextureMapMode_Wrap:
                        format.setWrapS(GL_REPEAT);
                        break;

                    case aiTextureMapMode_Clamp:
                        format.setWrapS(GL_CLAMP_TO_EDGE);
                        break;

                    case aiTextureMapMode_Decal:
                        format.setWrapS(GL_CLAMP_TO_EDGE);
                        break;

                    case aiTextureMapMode_Mirror:
                        format.setWrapS(GL_MIRRORED_REPEAT);
                        break;
                    }
                }
                int vwrap;
                if (AI_SUCCESS == mtl->Get(AI_MATKEY_MAPPINGMODE_V_DIFFUSE(0), vwrap))
                {
                    switch (vwrap)
                    {
                    case aiTextureMapMode_Wrap:
                        format.setWrapT(GL_REPEAT);
                        break;

                    case aiTextureMapMode_Clamp:
                        format.setWrapT(GL_CLAMP_TO_EDGE);
                        break;

                    case aiTextureMapMode_Decal:
                        format.setWrapT(GL_CLAMP_TO_EDGE);
                        break;

                    case aiTextureMapMode_Mirror:
                        format.setWrapT(GL_MIRRORED_REPEAT);
                        break;
                    }
                }

                bool isAsync = false;
                meshRef->mTextures[type] = am::texture2d(realPath.string(), format, isAsync);
            }
        }

        meshRef->setupFromAiMesh(mesh);
        meshRef->mValidCache = true;
        meshRef->mAnimatedPos.resize(mesh->mNumVertices);
        if (mesh->HasNormals())
        {
            meshRef->mAnimatedNorm.resize(mesh->mNumVertices);
        }

        meshRef->mIndices.resize(mesh->mNumFaces * 3);
        unsigned j = 0;
        for (unsigned x = 0; x < mesh->mNumFaces; ++x)
        {
            for (unsigned a = 0; a < mesh->mFaces[x].mNumIndices; ++a)
            {
                meshRef->mIndices[j++] = mesh->mFaces[x].mIndices[a];
            }
        }

        return meshRef;
    }

    void Scene::setupSceneMeshes()
    {
        CI_LOG_I("loading model " << mFilePath.filename().string() <<
            " [" << mFilePath.string() << "] ");
        for (unsigned i = 0; i < mAiScene->mNumMeshes; ++i)
        {
            string name = fromAssimp(mAiScene->mMeshes[i]->mName);
            CI_LOG_I("loading mesh " << i << ": " << name);
            MeshRef meshRef = convertAiMesh(mAiScene->mMeshes[i]);
            mAllMeshes.push_back(meshRef);
        }

#if 0
        animationTime = -1;
        setNormalizedTime(0);
#endif

        CI_LOG_I("finished loading model " << mFilePath.filename().string());
    }

    void Scene::updateAnimation(size_t animationIndex, double currentTime)
    {
        if (mAiScene->mNumAnimations == 0)
            return;

        const aiAnimation *mAnim = mAiScene->mAnimations[animationIndex];
        double ticks = mAnim->mTicksPerSecond;
        if (ticks == 0.0)
            ticks = 1.0;
        currentTime *= ticks;

        // calculate the transformations for each animation channel
        for (unsigned int a = 0; a < mAnim->mNumChannels; a++)
        {
            const aiNodeAnim *channel = mAnim->mChannels[a];

            MeshNodeRef targetNode = getAssimpNode(fromAssimp(channel->mNodeName));

            // ******** Position *****
            aiVector3D presentPosition(0, 0, 0);
            if (channel->mNumPositionKeys > 0)
            {
                // Look for present frame number. Search from last position if time is after the last time, else from beginning
                // Should be much quicker than always looking from start for the average use case.
                unsigned int frame = 0;// (currentTime >= lastAnimationTime) ? lastFramePositionIndex : 0;
                while (frame < channel->mNumPositionKeys - 1)
                {
                    if (currentTime < channel->mPositionKeys[frame + 1].mTime)
                        break;
                    frame++;
                }

                // interpolate between this frame's value and next frame's value
                unsigned int nextFrame = (frame + 1) % channel->mNumPositionKeys;
                const aiVectorKey& key = channel->mPositionKeys[frame];
                const aiVectorKey& nextKey = channel->mPositionKeys[nextFrame];
                double diffTime = nextKey.mTime - key.mTime;
                if (diffTime < 0.0)
                    diffTime += mAnim->mDuration;
                if (diffTime > 0)
                {
                    float factor = float((currentTime - key.mTime) / diffTime);
                    presentPosition = key.mValue + (nextKey.mValue - key.mValue) * factor;
                }
                else
                {
                    presentPosition = key.mValue;
                }
            }

            // ******** Rotation *********
            aiQuaternion presentRotation(1, 0, 0, 0);
            if (channel->mNumRotationKeys > 0)
            {
                unsigned int frame = 0;//(currentTime >= lastAnimationTime) ? lastFrameRotationIndex : 0;
                while (frame < channel->mNumRotationKeys - 1)
                {
                    if (currentTime < channel->mRotationKeys[frame + 1].mTime)
                        break;
                    frame++;
                }

                // interpolate between this frame's value and next frame's value
                unsigned int nextFrame = (frame + 1) % channel->mNumRotationKeys;
                const aiQuatKey& key = channel->mRotationKeys[frame];
                const aiQuatKey& nextKey = channel->mRotationKeys[nextFrame];
                double diffTime = nextKey.mTime - key.mTime;
                if (diffTime < 0.0)
                    diffTime += mAnim->mDuration;
                if (diffTime > 0)
                {
                    float factor = float((currentTime - key.mTime) / diffTime);
                    aiQuaternion::Interpolate(presentRotation, key.mValue, nextKey.mValue, factor);
                }
                else
                {
                    presentRotation = key.mValue;
                }
            }

            // ******** Scaling **********
            aiVector3D presentScaling(1, 1, 1);
            if (channel->mNumScalingKeys > 0)
            {
                unsigned int frame = 0;//(currentTime >= lastAnimationTime) ? lastFrameScaleIndex : 0;
                while (frame < channel->mNumScalingKeys - 1)
                {
                    if (currentTime < channel->mScalingKeys[frame + 1].mTime)
                        break;
                    frame++;
                }

                // TODO: (thom) interpolation maybe? This time maybe even logarithmic, not linear
                presentScaling = channel->mScalingKeys[frame].mValue;
            }

            targetNode->setRotation(fromAssimp(presentRotation));
            targetNode->setScale(fromAssimp(presentScaling));
            targetNode->setPosition(fromAssimp(presentPosition));
        }
    }

    MeshNodeRef Scene::getAssimpNode(const std::string &name)
    {
        auto i = mNodeMap.find(name);
        if (i != mNodeMap.end())
            return i->second;
        else
            return MeshNodeRef();
    }

    size_t Scene::getNumAnimations() const
    {
        return mAiScene->mNumAnimations;
    }

    void Scene::setAnimation(size_t n)
    {
        mAnimationIndex = math< size_t >::clamp(n, 0, getNumAnimations());
    }

    void Scene::setTime(double t)
    {
        mAnimationTime = t;
    }

    double Scene::getAnimationDuration(size_t n) const
    {
        if (mAiScene->mNumAnimations == 0)
            return 0;

        const aiAnimation *anim = mAiScene->mAnimations[n];
        double ticks = anim->mTicksPerSecond;
        if (ticks == 0.0)
            ticks = 1.0;
        return anim->mDuration / ticks;
    }

    void Scene::updateSkinning()
    {
        for (const auto& nodeRef : mAllNodes)
        {
            for (const auto& meshRef : nodeRef->mMeshes)
            {
                // current mesh we are introspecting
                const aiMesh *mesh = meshRef->mAiMesh;

                // calculate bone matrices
                std::vector< aiMatrix4x4 > boneMatrices(mesh->mNumBones);
                for (unsigned a = 0; a < mesh->mNumBones; ++a)
                {
                    const aiBone *bone = mesh->mBones[a];

                    // find the corresponding node by again looking recursively through
                    // the node hierarchy for the same name
                    MeshNodeRef nodeRef = getAssimpNode(fromAssimp(bone->mName));
                    assert(nodeRef);
                    // start with the mesh-to-bone matrix
                    // and append all node transformations down the parent chain until
                    // we're back at mesh coordinates again
                    boneMatrices[a] = toAssimp(nodeRef->getWorldTransform()) *
                        bone->mOffsetMatrix;
                }

                meshRef->mValidCache = false;

                meshRef->mAnimatedPos.resize(meshRef->mAnimatedPos.size());
                if (mesh->HasNormals())
                {
                    meshRef->mAnimatedNorm.resize(meshRef->mAnimatedNorm.size());
                }

                // loop through all vertex weights of all bones
                for (unsigned a = 0; a < mesh->mNumBones; ++a)
                {
                    const aiBone *bone = mesh->mBones[a];
                    const aiMatrix4x4 &posTrafo = boneMatrices[a];

                    for (unsigned b = 0; b < bone->mNumWeights; ++b)
                    {
                        const aiVertexWeight &weight = bone->mWeights[b];
                        size_t vertexId = weight.mVertexId;
                        const aiVector3D& srcPos = mesh->mVertices[vertexId];

                        meshRef->mAnimatedPos[vertexId] += weight.mWeight * (posTrafo * srcPos);
                    }

                    if (mesh->HasNormals())
                    {
                        // 3x3 matrix, contains the bone matrix without the
                        // translation, only with rotation and possibly scaling
                        aiMatrix3x3 normTrafo = aiMatrix3x3(posTrafo);
                        for (size_t b = 0; b < bone->mNumWeights; ++b)
                        {
                            const aiVertexWeight &weight = bone->mWeights[b];
                            size_t vertexId = weight.mVertexId;

                            const aiVector3D& srcNorm = mesh->mNormals[vertexId];
                            meshRef->mAnimatedNorm[vertexId] += weight.mWeight * (normTrafo * srcNorm);
                        }
                    }
                }
            }
        }
    }

    void Scene::updateMeshes()
    {
        for (const auto& nodeRef : mAllNodes)
        {
            for (const auto& meshRef : nodeRef->mMeshes)
            {
                if (meshRef->mValidCache)
                    continue;

                auto numVertices = meshRef->mCachedTriMesh->getNumVertices();

                if (mSkinningEnabled)
                {
                    // animated data
                    vec3* vertices = meshRef->mCachedTriMesh->getPositions<3>();
                    for (size_t v = 0; v < numVertices; ++v)
                        vertices[v] = fromAssimp(meshRef->mAnimatedPos[v]);

                    std::vector< vec3 > &normals = meshRef->mCachedTriMesh->getNormals();
                    for (size_t v = 0; v < normals.size(); ++v)
                        normals[v] = fromAssimp(meshRef->mAnimatedNorm[v]);
                }
                else
                {
                    // original mesh data from assimp
                    const aiMesh *mesh = meshRef->mAiMesh;

                    vec3* vertices = meshRef->mCachedTriMesh->getPositions<3>();
                    for (size_t v = 0; v < numVertices; ++v)
                        vertices[v] = fromAssimp(mesh->mVertices[v]);

                    std::vector< vec3 > &normals = meshRef->mCachedTriMesh->getNormals();
                    for (size_t v = 0; v < normals.size(); ++v)
                        normals[v] = fromAssimp(mesh->mNormals[v]);
                }

                meshRef->mValidCache = true;
            }
        }
    }

    void Scene::enableSkinning(bool enable /* = true */)
    {
        if (mSkinningEnabled == enable)
            return;

        mSkinningEnabled = enable;
        // invalidate mesh cache
        for (const auto& nodeRef : mAllNodes)
        {
            for (const auto& meshRef : nodeRef->mMeshes)
            {
                meshRef->mValidCache = false;
            }
        }
    }

    void Scene::update(double elapsed)
    {
        if (mAnimationEnabled)
            updateAnimation(mAnimationIndex, mAnimationTime);

        if (mSkinningEnabled)
            updateSkinning();

        updateMeshes();
    }

    void MeshNode::draw()
    {
        for (const auto& meshRef : mMeshes)
        {
            meshRef->draw();
        }
    }

    void Mesh::draw()
    {
        vector<shared_ptr<gl::ScopedTextureBind>> scopedTexBinds;
        map<int, int> texSemanticTable =
        {
            { aiTextureType_DIFFUSE, 0 },   // Base
            { aiTextureType_NORMALS, 1 },   // Normal
            { aiTextureType_EMISSIVE, 2 },  // Emissive
            { aiTextureType_UNKNOWN, 3 },   // Metal-roughness
            { aiTextureType_LIGHTMAP, 4 },  // AO
        };

        for (auto& kv : texSemanticTable)
        {
            if (mTextures[kv.first])
            {
                scopedTexBinds.emplace_back(make_shared<gl::ScopedTextureBind>(
                    mTextures[kv.first], kv.second));
            }
        }

        if (blendMode == BlendDefault)
        {
            gl::enableAlphaBlending();
        }
        else if (blendMode == BlendAdditive)
        {
            gl::enableAdditiveBlending();
        }
        else
        {
            gl::disableAlphaBlending();
        }

        if (mTwoSided)
            gl::disable(GL_CULL_FACE);
        else
            gl::enable(GL_CULL_FACE);

        gl::draw(*mCachedTriMesh);
    }

    void Scene::updateShaderDef(ShaderDefine& shaderDef)
    {
        for (auto& meshRef : mAllMeshes)
        {
            if (meshRef->mTextures[aiTextureType_DIFFUSE]) shaderDef.HAS_BASECOLORMAP = true;
            if (meshRef->mTextures[aiTextureType_NORMALS]) shaderDef.HAS_NORMALMAP = true;
            if (meshRef->mTextures[aiTextureType_EMISSIVE]) shaderDef.HAS_EMISSIVEMAP = true;
            if (meshRef->mTextures[aiTextureType_UNKNOWN]) shaderDef.HAS_METALROUGHNESSMAP = true;
            if (meshRef->mTextures[aiTextureType_LIGHTMAP]) shaderDef.HAS_OCCLUSIONMAP = true;

            if (meshRef->meshFormat.mTexCoords0Dims > 0) shaderDef.HAS_UV = true;
            if (meshRef->meshFormat.mNormalsDims > 0) shaderDef.HAS_NORMALS = true;
            if (meshRef->meshFormat.mTangentsDims > 0) shaderDef.HAS_TANGENTS = true;
        }
    }

} // namespace assimp

