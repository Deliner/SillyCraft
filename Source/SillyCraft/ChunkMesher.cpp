// Fill out your copyright notice in the Description page of Project Settings.

#include "ChunkMesher.h"

ChunkMesher::ChunkMesher(BlockRegistry* registry) : m_registry(registry)
{
}

void ChunkMesher::MeshChunk(const AChunk* chunk) const
{
	std::array<Voxel, Constants::ChunkSize3D> voxels({});

	TArray<FVector>* vectors = new TArray<FVector>;
	TArray<int32>* indice = new TArray<int32>;
	TArray<FLinearColor>* color = new TArray<FLinearColor>;
	std::vector<Voxel*>* meshedVoxels = new std::vector<Voxel*>;
	NaiveGreedyMeshing(voxels, chunk);
	GreedyMeshing(voxels, *meshedVoxels);

	int index = 0;

	for (int i = 0; i < meshedVoxels->size(); i++)
	{
		Voxel* voxel = (*meshedVoxels)[i];
		Block* currentBlock = m_registry->GetBlock(voxel->ID);
		if (currentBlock->BlockHardness != Block::Hardness::Empty) {
			for (int f = 0; f < 6; f++) {
				Face* face = &voxel->face[f];
				if (face->visible) {

					IntVector* v1 = &face->v1;
					IntVector* v2 = &face->v2;
					IntVector* v3 = &face->v3;
					IntVector* v4 = &face->v4;

					vectors->Add(v1->GetVertice());
					vectors->Add(v2->GetVertice());
					vectors->Add(v3->GetVertice());
					vectors->Add(v4->GetVertice());

					AddIndice(index, *indice);
					index += 4;

					color->Add(currentBlock->Color);
					color->Add(currentBlock->Color);
					color->Add(currentBlock->Color);
					color->Add(currentBlock->Color);
				}
			}
		}
	}

	chunk->Mesh->CreateMeshSection_LinearColor(int32(0), *vectors, *indice, TArray<FVector>(), TArray<FVector2D>(), *color, TArray<FProcMeshTangent>(), false);

	delete vectors;
	delete indice;
	delete color;
	delete meshedVoxels;
}

FVector ChunkMesher::IntVector::GetVertice() const
{
	float x = X * Constants::ChunkScale;
	float y = Y * Constants::ChunkScale;
	float z = Z * Constants::ChunkScale;
	return FVector(x, y, z);
}

void ChunkMesher::NaiveGreedyMeshing(std::array<Voxel, Constants::ChunkSize3D>& voxels, const AChunk* chunk) const
{
	IntVector* v1 = nullptr;
	IntVector* v2 = nullptr;
	IntVector* v3 = nullptr;
	IntVector* v4 = nullptr;
	Face* face = nullptr;
	Face* prevface = nullptr;
	int index = 0;

	for (int i = 0; i < Constants::ChunkSize3D; i++)
	{
		int id = chunk->GetBlockID(i);
		Block* currentBlock = m_registry->GetBlock(id);
		Voxel* voxel = &voxels[i];
		voxel->ID = id;

		if (currentBlock->BlockHardness != Block::Hardness::Empty)
		{
			int y = i % Constants::ChunkSize;
			int z = (i / Constants::ChunkSize) % Constants::ChunkSize;
			int x = i / Constants::ChunkSize2D;

			//Front
			face = &voxel->face[Faces::Front];
			id = m_registry->Air->ID;

			if (x > 0) {
				index = i + MakeIndex(0, 0, -1);
				id = voxels[index].ID;
			}

			if (id != currentBlock->ID)
			{
				face->visible = true;

				v1 = &face->v1;
				v2 = &face->v2;
				v3 = &face->v3;
				v4 = &face->v4;

				v2->Y = y + 1;
				v2->Z = z;
				v2->X = x;

				v3->Y = y + 1;
				v3->Z = z + 1;
				v3->X = x;

				id = m_registry->Air->ID;
				if (y > 0) {
					index = i + MakeIndex(-1, 0, 0);
					id = chunk->GetBlockID(index);
				}

				prevface = &voxels[index].face[Faces::Front];

				if (id == voxel->ID && prevface->visible)
				{
					ChunkMesher::Join(face, prevface);
				}
				else
				{
					v1->Y = y;
					v1->Z = z;
					v1->X = x;

					v4->Y = y;
					v4->Z = z + 1;
					v4->X = x;
				}
			}
			else
			{
				face->visible = false;
			}

			//Back
			face = &voxel->face[Faces::Back];
			id = m_registry->Air->ID;

			if (x < Constants::ChunkSize - 1) {
				index = i + MakeIndex(0, 0, 1);
				id = chunk->GetBlockID(index);
			}

			if (id != currentBlock->ID)
			{
				face->visible = true;

				v1 = &face->v1;
				v2 = &face->v2;
				v3 = &face->v3;
				v4 = &face->v4;

				v1->Y = y + 1;
				v1->Z = z;
				v1->X = x + 1;

				v4->Y = y + 1;
				v4->Z = z + 1;
				v4->X = x + 1;

				id = m_registry->Air->ID;
				if (y > 0) {
					index = i + MakeIndex(-1, 0, 0);
					id = chunk->GetBlockID(index);
				}

				prevface = &voxels[index].face[Faces::Back];

				if (id == voxel->ID && prevface->visible)
				{
					ChunkMesher::JoinReversed(face, prevface);
				}
				else
				{
					v2->Y = y;
					v2->Z = z;
					v2->X = x + 1;

					v3->Y = y;
					v3->Z = z + 1;
					v3->X = x + 1;
				}
			}
			else
			{
				face->visible = false;
			}

			//Left
			face = &voxel->face[Faces::Left];
			id = m_registry->Air->ID;

			if (y > 0) {
				index = i + MakeIndex(-1, 0, 0);
				id = chunk->GetBlockID(index);
			}

			if (id != currentBlock->ID)
			{
				face->visible = true;

				v1 = &face->v1;
				v2 = &face->v2;
				v3 = &face->v3;
				v4 = &face->v4;

				v1->Y = y;
				v1->Z = z;
				v1->X = x + 1;

				v4->Y = y;
				v4->Z = z + 1;
				v4->X = x + 1;

				id = m_registry->Air->ID;
				if (x > 0) {
					index = i + MakeIndex(0, 0, -1);
					id = chunk->GetBlockID(index);
				}

				prevface = &voxels[index].face[Faces::Left];

				if (id == voxel->ID && prevface->visible)
				{
					ChunkMesher::JoinReversed(face, prevface);
				}
				else
				{
					v2->Y = y;
					v2->Z = z;
					v2->X = x;

					v3->Y = y;
					v3->Z = z + 1;
					v3->X = x;
				}
			}
			else
			{
				face->visible = false;
			}

			//Right
			face = &voxel->face[Faces::Right];
			id = m_registry->Air->ID;

			if (y < Constants::ChunkSize - 1) {
				index = i + MakeIndex(1, 0, 0);
				id = chunk->GetBlockID(index);
			}

			if (id != currentBlock->ID)
			{
				face->visible = true;

				v1 = &face->v1;
				v2 = &face->v2;
				v3 = &face->v3;
				v4 = &face->v4;

				v2->Y = y + 1;
				v2->Z = z;
				v2->X = x + 1;

				v3->Y = y + 1;
				v3->Z = z + 1;
				v3->X = x + 1;

				id = m_registry->Air->ID;
				if (x > 0)
				{
					index = i + MakeIndex(0, 0, -1);
					id = chunk->GetBlockID(index);
				}

				prevface = &voxels[index].face[Faces::Right];

				if (id == voxel->ID && prevface->visible)
				{
					ChunkMesher::Join(face, prevface);
				}
				else
				{
					v1->Y = y + 1;
					v1->Z = z;
					v1->X = x;

					v4->Y = y + 1;
					v4->Z = z + 1;
					v4->X = x;
				}
			}
			else
			{
				face->visible = false;
			}

			//Bottom
			face = &voxel->face[Faces::Bottom];
			id = m_registry->Air->ID;

			if (z > 0) {
				index = i + MakeIndex(0, -1, 0);
				id = chunk->GetBlockID(index);
			}

			if (id != currentBlock->ID)
			{
				face->visible = true;

				v1 = &face->v1;
				v2 = &face->v2;
				v3 = &face->v3;
				v4 = &face->v4;

				v1->Y = y + 1;
				v1->Z = z;
				v1->X = x;

				v4->Y = y + 1;
				v4->Z = z;
				v4->X = x + 1;

				id = m_registry->Air->ID;
				if (y > 0) {
					index = i + MakeIndex(-1, 0, 0);
					id = chunk->GetBlockID(index);
				}

				prevface = &voxels[index].face[Faces::Bottom];

				if (id == voxel->ID && prevface->visible)
				{
					ChunkMesher::JoinReversed(face, prevface);
				}
				else
				{
					v2->Y = y;
					v2->Z = z;
					v2->X = x;

					v3->Y = y;
					v3->Z = z;
					v3->X = x + 1;
				}
			}
			else
			{
				face->visible = false;
			}

			//Top
			face = &voxel->face[Faces::Top];
			id = m_registry->Air->ID;

			if (z < Constants::ChunkSize - 1) {
				index = i + MakeIndex(0, 1, 0);
				id = chunk->GetBlockID(index);
			}

			if (id != currentBlock->ID)
			{
				face->visible = true;

				v1 = &face->v1;
				v2 = &face->v2;
				v3 = &face->v3;
				v4 = &face->v4;

				v2->Y = y + 1;
				v2->Z = z + 1;
				v2->X = x;

				v3->Y = y + 1;
				v3->Z = z + 1;
				v3->X = x + 1;

				id = m_registry->Air->ID;

				if (y > 0) {
					index = i + MakeIndex(-1, 0, 0);
					id = chunk->GetBlockID(index);
				}

				prevface = &voxels[index].face[Faces::Top];

				if (id == voxel->ID && prevface->visible)
				{
					ChunkMesher::Join(face, prevface);
				}
				else
				{
					v1->Y = y;
					v1->Z = z + 1;
					v1->X = x;

					v4->Y = y;
					v4->Z = z + 1;
					v4->X = x + 1;
				}
			}
			else
			{
				face->visible = false;
			}
		}
	}
}

void ChunkMesher::GreedyMeshing(std::array<Voxel, Constants::ChunkSize3D>& voxels, std::vector<Voxel*>& meshedVoxels) const
{
	int index = 0;

	for (int i = 0; i < Constants::ChunkSize3D; i++) 
	{
		int y = i % Constants::ChunkSize;
		int z = (i / Constants::ChunkSize) % Constants::ChunkSize;
		int x = i / Constants::ChunkSize2D;

		bool valid = false;
		Voxel* voxel = &voxels[i];

		for (int f = 0; f < 6; f++) 
		{
			Face* face = &voxel->face[f];
			if(face->visible){
				switch (f)
				{
				case Faces::Front:
				case Faces::Back:
				case Faces::Right:
				case Faces::Left:
					if (y < Constants::ChunkSize - 1) {
						index = i + MakeIndex(0, 1, 0);
						Face* otherface = &voxels[index].face[f];
						if (ChunkMesher::GreedyJoin(face, otherface)) 
						{
							continue;
						}
					}
					break;
				case Faces::Top:
				case Faces::Bottom:
					if (x < Constants::ChunkSize - 1) {
						index = i + MakeIndex(0, 0, 1);
						Face* otherface = &voxels[index].face[f];
						if (ChunkMesher::GreedyJoin(face, otherface))
						{
							continue;
						}
					}
					break;
				}
				valid = true;
			}
		}
		if(valid)
		{
			meshedVoxels.push_back(voxel);
		}
	}
}

void ChunkMesher::Join(Face* face, Face* prevface)
{
	if (prevface->prevFace)
	{
		prevface->visible = false;
		prevface = prevface->prevFace;
	}

	face->v1 = prevface->v1;
	face->v4 = prevface->v4;

	prevface->visible = false;
	face->prevFace = prevface;
}

void ChunkMesher::JoinReversed(Face* face, Face* prevface)
{
	if (prevface->prevFace)
	{
		prevface->visible = false;
		prevface = prevface->prevFace;
	}

	face->v2 = prevface->v2;
	face->v3 = prevface->v3;

	prevface->visible = false;
	face->prevFace = prevface;
}

bool ChunkMesher::GreedyJoin(Face* face, Face* otherface)
{
	if (face->v3 == otherface->v2 && face->v4 == otherface->v1 && otherface->visible)
	{
		otherface->v1 = face->v1;
		otherface->v2 = face->v2;
		face->visible = false;
		return true;
	}
	return false;
}

void ChunkMesher::AddIndice(const int& index, TArray<int>& indice) const
{
	indice.Add(index);
	indice.Add(index + 1);
	indice.Add(index + 2);


	indice.Add(index + 2);
	indice.Add(index + 3);
	indice.Add(index);
}

int ChunkMesher::MakeIndex(const int& y, const int& z, const int& x) const
{
	return y + z * Constants::ChunkSize + x * Constants::ChunkSize2D;
}

ChunkMesher::~ChunkMesher()
{
}

bool ChunkMesher::IntVector:: operator ==(const IntVector& a) 
{
	return X == a.X && Y == a.Y && Z == a.Z;
}
bool ChunkMesher::IntVector:: operator !=(const IntVector& a)
{
	return !(*this == a);
}