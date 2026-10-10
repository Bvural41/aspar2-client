// Granny 3D Animation Complete Stubs for Android ARM64 and x86_64
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include "granny.h"

extern "C" {

static granny_data_type_definition s_pnt332_def;
granny_data_type_definition* GrannyPNT332VertexType = &s_pnt332_def;
granny_data_type_definition* GrannyPNG333VertexType = &s_pnt332_def;
granny_data_type_definition* GrannyPNGT3332VertexType = &s_pnt332_def;
granny_data_type_definition* GrannyPNGBT33332VertexType = &s_pnt332_def;
granny_data_type_definition* GrannyFileInfoType = &s_pnt332_def;

static granny_file_info s_empty_file_info;

// File loading
granny_file* GrannyReadEntireFileFromMemory(granny_int32x MemorySize, void const* Memory)
{
    return (granny_file*)calloc(1, 128);
}

void GrannyFreeFile(granny_file* File)
{
    if (File) free(File);
}

void GrannyFreeFileSection(granny_file* File, granny_int32x SectionIndex)
{
}

granny_file_info* GrannyGetFileInfo(granny_file* File)
{
    return &s_empty_file_info;
}

// Local Pose
granny_local_pose* GrannyNewLocalPose(granny_int32x BoneCount)
{
    return (granny_local_pose*)calloc(1, sizeof(float) * 16 * (BoneCount > 0 ? BoneCount : 1) + 64);
}

void GrannyFreeLocalPose(granny_local_pose* LocalPose)
{
    if (LocalPose) free(LocalPose);
}

// Mesh binding & deformer
granny_mesh_binding* GrannyNewMeshBinding(granny_mesh const* Mesh,
                                          granny_skeleton const* FromSkeleton,
                                          granny_skeleton const* ToSkeleton)
{
    return (granny_mesh_binding*)calloc(1, 64);
}

void GrannyFreeMeshBinding(granny_mesh_binding* Binding)
{
    if (Binding) free(Binding);
}

granny_mesh_deformer* GrannyNewMeshDeformer(granny_data_type_definition const* InputVertexLayout,
                                            granny_data_type_definition const* OutputVertexLayout,
                                            granny_deformation_type DeformationType,
                                            granny_deformer_tail_flags TailFlag)
{
    return (granny_mesh_deformer*)calloc(1, 64);
}

void GrannyFreeMeshDeformer(granny_mesh_deformer* Deformer)
{
    if (Deformer) free(Deformer);
}

bool GrannyMeshIsRigid(granny_mesh const* Mesh)
{
    return true;
}

granny_int32x GrannyGetMeshTriangleGroupCount(granny_mesh const* Mesh)
{
    return 0;
}

granny_tri_material_group* GrannyGetMeshTriangleGroups(granny_mesh const* Mesh)
{
    return NULL;
}

granny_data_type_definition* GrannyGetMeshVertexType(granny_mesh const* Mesh)
{
    return GrannyPNT332VertexType;
}

granny_int32x GrannyGetMeshVertexCount(granny_mesh const* Mesh)
{
    return 0;
}

granny_int32x GrannyGetMeshIndexCount(granny_mesh const* Mesh)
{
    return 0;
}

void* GrannyGetMeshVertices(granny_mesh const* Mesh)
{
    return NULL;
}

void GrannyCopyMeshVertices(granny_mesh const* Mesh,
                            granny_data_type_definition const* VertexType,
                            void* DestVertices)
{
}

void GrannyCopyMeshIndices(granny_mesh const* Mesh,
                           granny_int32x BytesPerIndex,
                           void* DestIndices)
{
}

void GrannyDeformVertices(granny_mesh_deformer const* Deformer,
                          granny_int32x const* MatrixIndices,
                          granny_real32 const* MatrixBuffer4x4,
                          granny_int32x VertexCount,
                          void const* SourceVertices,
                          void* DestVertices)
{
}

granny_int32x const* GrannyGetMeshBindingToBoneIndices(granny_mesh_binding const* Binding)
{
    static granny_int32x s_zero = 0;
    return &s_zero;
}

granny_texture* GrannyGetMaterialTextureByType(granny_material const* Material,
                                              granny_material_texture_type Type)
{
    return NULL;
}

bool GrannyFindMatchingMember(granny_data_type_definition const* SourceType,
                              void const* SourceObject,
                              char const* DestMemberName,
                              granny_variant* Result)
{
    return false;
}

void GrannyConvertSingleObject(granny_data_type_definition const* SourceType,
                               void const* SourceObject,
                               granny_data_type_definition const* DestType,
                               void* DestObject,
                               granny_conversion_handler* OverrideHandler)
{
}

granny_int32x GrannyGetTotalTypeSize(granny_data_type_definition const* TypeDefinition)
{
    return 32;
}

// Model instance
granny_model_instance* GrannyInstantiateModel(granny_model const* Model)
{
    return (granny_model_instance*)calloc(1, 64);
}

void GrannyFreeModelInstance(granny_model_instance* ModelInstance)
{
    if (ModelInstance) free(ModelInstance);
}

granny_skeleton* GrannyGetSourceSkeleton(granny_model_instance const* Model)
{
    return NULL;
}

void GrannySetModelClock(granny_model_instance const* ModelInstance, granny_real32 NewClock)
{
}

void GrannyFreeCompletedModelControls(granny_model_instance const* ModelInstance)
{
}

void GrannySampleModelAnimations(granny_model_instance const* ModelInstance,
                                 granny_int32x FirstBone,
                                 granny_int32x BoneCount,
                                 granny_local_pose* Result)
{
}

void GrannySampleModelAnimationsAccelerated(granny_model_instance const* ModelInstance,
                                            granny_int32x BoneCount,
                                            granny_real32 const* Offset4x4,
                                            granny_local_pose* Result,
                                            granny_world_pose* WorldPoseResult)
{
}

void GrannyUpdateModelMatrix(granny_model_instance const* ModelInstance,
                             granny_real32 SecondsElapsed,
                             granny_real32 const* ModelMatrix4x4,
                             granny_real32* Result4x4,
                             bool Inverse)
{
}

// Animation controls
void GrannyFreeControl(granny_control* Control)
{
    if (Control) free(Control);
}

void GrannyFreeControlOnceUnused(granny_control* Control)
{
    if (Control) free(Control);
}

void GrannyCompleteControlAt(granny_control* Control, granny_real32 AtSeconds)
{
}

bool GrannyControlIsComplete(granny_control const* Control)
{
    return false;
}

bool GrannyFreeControlIfComplete(granny_control* Control)
{
    return false;
}

granny_control* GrannyPlayControlledAnimation(granny_real32 StartTime,
                                              granny_animation const* Animation,
                                              granny_model_instance* ModelInstance)
{
    return (granny_control*)calloc(1, 64);
}

granny_int32x GrannyGetControlLoopCount(granny_control const* Control)
{
    return 0;
}

void GrannySetControlLoopCount(granny_control* Control, granny_int32x LoopCount)
{
}

granny_real32 GrannyGetControlSpeed(granny_control const* Control)
{
    return 1.0f;
}

void GrannySetControlSpeed(granny_control* Control, granny_real32 Speed)
{
}

void GrannySetControlEaseIn(granny_control* Control, bool EaseIn)
{
}

void GrannySetControlEaseInCurve(granny_control* Control,
                                 granny_real32 StartSeconds,
                                 granny_real32 EndSeconds,
                                 granny_real32 StartValue,
                                 granny_real32 StartTangent,
                                 granny_real32 EndTangent,
                                 granny_real32 EndValue)
{
}

void GrannySetControlEaseOut(granny_control* Control, bool EaseOut)
{
}

void GrannySetControlEaseOutCurve(granny_control* Control,
                                  granny_real32 StartSeconds,
                                  granny_real32 EndSeconds,
                                  granny_real32 StartValue,
                                  granny_real32 StartTangent,
                                  granny_real32 EndTangent,
                                  granny_real32 EndValue)
{
}

granny_real32 GrannyGetControlLocalDuration(granny_control const* Control)
{
    return 1.0f;
}

granny_real32 GrannyGetControlDurationLeft(granny_control* Control)
{
    return 0.0f;
}

granny_real32 GrannyGetControlRawLocalClock(granny_control* Control)
{
    return 0.0f;
}

void GrannySetControlRawLocalClock(granny_control* Control, granny_real32 LocalClock)
{
}

// Bones & World Pose
bool GrannyFindBoneByName(granny_skeleton const* Skeleton, char const* BoneName, granny_int32x* BoneIndex)
{
    if (BoneIndex) *BoneIndex = 0;
    return false;
}

granny_world_pose* GrannyNewWorldPose(granny_int32x BoneCount)
{
    return (granny_world_pose*)calloc(1, sizeof(float) * 16 * (BoneCount > 0 ? BoneCount : 1) + 64);
}

void GrannyFreeWorldPose(granny_world_pose* WorldPose)
{
    if (WorldPose) free(WorldPose);
}

void GrannyBuildWorldPose(granny_skeleton const* Skeleton,
                          granny_int32x FirstBone,
                          granny_int32x BoneCount,
                          granny_local_pose const* LocalPose,
                          granny_real32 const* Offset4x4,
                          granny_world_pose* Result)
{
}

static float s_identityMatrix[16] = {
    1,0,0,0,
    0,1,0,0,
    0,0,1,0,
    0,0,0,1
};

granny_real32* GrannyGetWorldPose4x4(granny_world_pose const* WorldPose, granny_int32x BoneIndex)
{
    return s_identityMatrix;
}

granny_real32* GrannyGetWorldPoseComposite4x4(granny_world_pose const* WorldPose, granny_int32x BoneIndex)
{
    return s_identityMatrix;
}

granny_matrix_4x4* GrannyGetWorldPose4x4Array(granny_world_pose const* WorldPose)
{
    return (granny_matrix_4x4*)s_identityMatrix;
}

granny_matrix_4x4* GrannyGetWorldPoseComposite4x4Array(granny_world_pose const* WorldPose)
{
    return (granny_matrix_4x4*)s_identityMatrix;
}

// Logging and Clock
void GrannySetLogCallback(granny_log_callback const* LogCallback)
{
}

void GrannyGetSystemSeconds(granny_system_clock* Result)
{
}

granny_real32 GrannyGetSecondsElapsed(granny_system_clock const* StartClock,
                                      granny_system_clock const* EndClock)
{
    return 0.0f;
}

} // extern "C"
