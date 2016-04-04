#include "PlayerFactory.hpp"
#include "Player.hpp"


PlayerFactory::PlayerFactory(const TextureHolder& textures)
	: mSceneNode(nullptr)
	, mPlayer(nullptr)
	, mTextures(textures)
	, mTimer(sf::Time::Zero)
	, mPosition()
	, mIsAlive(true)
	, mCanSpawn(true)
{
}

void PlayerFactory::setSceneNode(SceneNode* node)
{
	mSceneNode = node;
}

Player*	PlayerFactory::create(sf::Vector2f position) const
{
	auto player(std::make_unique<Player>((mCanSpawn) ? Type::BigPlayer : Type::SmallPlayer, mTextures));
	mPlayer = player.get();
	mPlayer->setPosition(position);
	mSceneNode->attachChild(std::move(player));

	return mPlayer;
}

Player* PlayerFactory::spawn() const
{
	if (mIsAlive)
	{
		if (mPlayer != nullptr && !mPlayer->isDestroyed())
			return mPlayer;
		else
			return nullptr;
	}

	mIsAlive = true;

	return create(mPosition);
}

bool PlayerFactory::update(sf::Time dt) const
{
	if (mPlayer != nullptr && (!mPlayer->isDestroyed() || !mIsAlive /*|| !mCanSpawn*/))
		return false;

	mPlayer->playerHitEffect();
	mTimer += dt;

	if (mTimer > sf::seconds(1.f))
	{
		mPlayer->setMarkToRemove();
		mPosition = mPlayer->getWorldPosition();

		mPosition.y -= 32.f; // one tile up

		mCanSpawn = false;
		mIsAlive = false;
		mTimer = sf::Time::Zero;
	}

	return true;
}