
#include "Enemies.h"

#include "Effects.h"
#include "Camera.h"
#include "Enemy.h"








Enemies::Enemies()
{







}


Enemies::~Enemies()
{
	

	for (UINT i = 0; i < enemyclass.size(); ++i)
	{
	
		delete enemyclass[i];


	}



	
}






void Enemies::draw(ID3D11DeviceContext* dc, Camera& camera, ID3DX11EffectTechnique* activeTexTech)
{



	XMMATRIX world;
	XMMATRIX worldInvTranspose;
	XMMATRIX worldViewProj;


	XMMATRIX view = camera.View();
	XMMATRIX proj = camera.Proj();
	XMMATRIX viewProj = camera.ViewProj();



	for (UINT modelIndex = 0; modelIndex < mEnemyInstances.size(); ++modelIndex)
	{
		world = XMLoadFloat4x4(&mEnemyInstances[modelIndex].World);
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldViewProj = world*view*proj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);




		//I do not have shadoews
		//Effects::BasicFX->SetShadowTransform(world*shadowTransform);
		Effects::BasicFX->SetTexTransform(XMMatrixScaling(1.0f, 1.0f, 1.0f));

		for (UINT subset = 0; subset < mEnemyInstances[modelIndex].Model->SubsetCount; ++subset)
		{
			Effects::BasicFX->SetMaterial(mEnemyInstances[modelIndex].Model->Mat[subset]);
			Effects::BasicFX->SetDiffuseMap(mEnemyInstances[modelIndex].Model->DiffuseMapSRV[subset]);
			//Effects::BasicFX->SetNormalMap(mModelInstances[modelIndex].Model->NormalMapSRV[subset]);

			activeTexTech->GetPassByIndex(0)->Apply(0, dc);
			mEnemyInstances[modelIndex].Model->ModelMesh.Draw(dc, subset);




		}

	}




}





void Enemies::addEnemy(BasicModelInstance theEnemy)
{



	mEnemyInstances.push_back(theEnemy);


}



void Enemies::createEnemy(ID3D11Device* device, TextureMgr& texMgr,
	const std::string& modelFilename,
	const std::wstring& texturePath, FLOAT x, FLOAT y, FLOAT z)
{
	Enemy* newEnemy;

	newEnemy = new Enemy();

	XMMATRIX modelScale = XMMatrixScaling(3.0f, 3.0f, -3.0f);
	XMMATRIX modelRot = XMMatrixRotationY(0);
	XMMATRIX modelOffset = XMMatrixTranslation(x,y,z);


	newEnemy->setModelScale(modelScale);
	newEnemy->setModelRot(modelRot);
	newEnemy->setModelOffset(modelOffset);


	anEnemy = new BasicModel(device, texMgr, modelFilename, texturePath);

	newEnemy->setModel(anEnemy);


	BasicModelInstance oneEnemy;


	theEnemy.Model = anEnemy;


	XMStoreFloat4x4(&theEnemy.World, modelScale*modelRot*modelOffset);

	
	newEnemy->setBasicMInstance(theEnemy);

	oneEnemy = newEnemy->getBasicMInstance();

	

	addEnemy(theEnemy);

	enemyclass.push_back(newEnemy);

	
	LevelCollisions.push_back(EnemyBox);

	newEnemy->setWorld(theEnemy.World);





}



void Enemies::CreateBoundingBox()
{


	//
	// Compute scene bounding box.
	//
	XMFLOAT3 minPt(+MathHelper::Infinity, +MathHelper::Infinity, +MathHelper::Infinity);
	XMFLOAT3 maxPt(-MathHelper::Infinity, -MathHelper::Infinity, -MathHelper::Infinity);



	for (UINT i = 0; i < mEnemyInstances.size(); ++i)
	{
		minPt.x = 0.0f;
		minPt.y = 0.0f;
		minPt.z = 0.0f;

		maxPt.x = 0.0f;
		maxPt.y = 0.0f;
		maxPt.z = 0.0f;

		for (UINT j = 0; j < mEnemyInstances[i].Model->BasicVertices.size(); ++j)
		{

			XMFLOAT3 P = mEnemyInstances[i].Model->BasicVertices[j].Pos;


			//////multiply all these by 7

			minPt.x = MathHelper::Min(minPt.x, P.x);
			minPt.y = MathHelper::Min(minPt.y, P.y);
			minPt.z = MathHelper::Min(minPt.z, P.z);

			maxPt.x = MathHelper::Max(maxPt.x, P.x);
			maxPt.y = MathHelper::Max(maxPt.y, P.y);
			maxPt.z = MathHelper::Max(maxPt.z, P.z);


		}

		XMMATRIX temp = XMLoadFloat4x4(&mEnemyInstances[i].World);

		enemyclass[i]->setWorld(mEnemyInstances[i].World);

		XMVECTOR Scale;
		XMVECTOR Position;
		XMVECTOR Rotation;


		XMMatrixDecompose(&Scale, &Rotation, &Position, temp);

		XMFLOAT3 tempPos;
		XMStoreFloat3(&tempPos, Position);

		LevelCollisions[i].Center = tempPos;


		LevelCollisions[i].Extents = XMFLOAT3(0.5f*(maxPt.x - minPt.x),
			0.5f*(maxPt.y - minPt.y),
			0.5f*(maxPt.z - minPt.z));

		LevelCollisions[i].collisionType = 1 ;

		LevelCollisions[i].Extents.x = LevelCollisions[i].Extents.x * 3;
		LevelCollisions[i].Extents.y = LevelCollisions[i].Extents.y * 3;
		LevelCollisions[i].Extents.z = LevelCollisions[i].Extents.z * 3;

		EnemyBox.collisionType = 1;

		
		enemyclass[i]->setAABB(&LevelCollisions[i]);

	}
}



std::vector<BasicModelInstance> Enemies::getEnemy()
{

	return mEnemyInstances;
}



std::vector <XNA::AxisAlignedBox> Enemies::getEnemyCollisions()
{

	

	



	return LevelCollisions;
}




void Enemies::update(float dt)
{
	

	
	DeltaTime = dt;

	for (int i = 0; i < mEnemyInstances.size(); i++)
	{

		enemyclass[i]->update(DeltaTime);
		
		mEnemyInstances[i].World = enemyclass[i]->GetWorld();
		
		XMMATRIX temp = XMLoadFloat4x4(&mEnemyInstances[i].World);
		XMVECTOR Scale;
		XMVECTOR Rotation;
		XMVECTOR Position;

		XMFLOAT3 tempPosition;

		XMMatrixDecompose(&Scale, &Rotation, &Position, temp);
		XMStoreFloat3(&tempPosition, Position);
	


		LevelCollisions[i].Center = tempPosition;


	}
	
}


void Enemies::RemovemObjectInstance(int number)
{

	mEnemyInstances.erase(mEnemyInstances.begin() + number);

	LevelCollisions.erase(LevelCollisions.begin() + number);
	delete(enemyclass[number]);
}