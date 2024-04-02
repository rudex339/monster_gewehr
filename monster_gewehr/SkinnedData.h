#ifndef SKINNEDDATA_H
#define SKINNEDDATA_H

#include "Common/d3dUtil.h"
#include "Common/MathHelper.h"

///<summary>
/// A Keyframe defines the bone transformation at an instant in time.
///</summary>
struct Keyframe
{
	Keyframe();
	~Keyframe();

    float TimePos;
	DirectX::XMFLOAT3 Translation;
    DirectX::XMFLOAT3 Scale;
    DirectX::XMFLOAT4 RotationQuat;
};

///<summary>
/// A BoneAnimation is defined by a list of keyframes.  For time
/// values inbetween two keyframes, we interpolate between the
/// two nearest keyframes that bound the time.  
///
/// We assume an animation always has two keyframes.
///</summary>
struct BoneAnimation
{
	float GetStartTime()const;
	float GetEndTime()const;

    void Interpolate(float t, DirectX::XMFLOAT4X4& M)const;

	std::vector<Keyframe> Keyframes; 	
};

///<summary>
/// AnimationClip�� ���� "Walk", "Run", "Attack", "Defend"�Դϴ�.
/// AnimationClip���� �ִϸ��̼� Ŭ���� �����ϱ� ���� ��� ���� ���� BoneAnimation�� �ʿ��մϴ�.    
///</summary>
struct AnimationClip
{
	float GetClipStartTime()const;
	float GetClipEndTime()const;

    void Interpolate(float t, std::vector<DirectX::XMFLOAT4X4>& boneTransforms)const;

    std::vector<BoneAnimation> BoneAnimations; 	
};

class SkinnedData
{
public:

	UINT BoneCount()const;

	float GetClipStartTime(const std::string& clipName)const;
	float GetClipEndTime(const std::string& clipName)const;

	void Set(
		std::vector<int>& boneHierarchy, 
		std::vector<DirectX::XMFLOAT4X4>& boneOffsets,
		std::unordered_map<std::string, AnimationClip>& animations);

	 // ���� ������Ʈ������ ������ Ŭ�� �̸��� ����Ͽ� ���ÿ� ���� �� ȣ���� ���ɼ��� �ִ� ��� ����� ĳ���ϰ� ���� ���Դϴ�.
    void GetFinalTransforms(const std::string& clipName, float timePos, 
		 std::vector<DirectX::XMFLOAT4X4>& finalTransforms)const;

private:
    // Gives parentIndex of ith bone.
	std::vector<int> mBoneHierarchy;

	std::vector<DirectX::XMFLOAT4X4> mBoneOffsets;
   
	std::unordered_map<std::string, AnimationClip> mAnimations;
};
 
#endif // SKINNEDDATA_H