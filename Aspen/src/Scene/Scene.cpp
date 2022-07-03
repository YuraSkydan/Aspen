#include "Scene.h"
#include "../Log/Log.h"
#include "../Math/Math.h"

#include "box2d/b2_polygon_shape.h"
#include "box2d/b2_fixture.h"
#include "box2d/b2_circle_shape.h"

void Scene::PhysicsWorldStart()
{
	for (auto& object : m_GameObjects)
	{
		Vector2f position = object->transform->position;
		float angle = ToRads(-object->transform->angle);

		b2BodyDef bodyDef;
		bodyDef.position = b2Vec2(position.x, position.y);
		bodyDef.angle = angle;

		b2Body* body = nullptr;
		if (object->HasComponent<Rigidbody>())
		{
			Rigidbody* rigidbody = object->GetComponent<Rigidbody>();
			bodyDef.type = b2BodyType(rigidbody->type);
			bodyDef.gravityScale = rigidbody->gravityScale;
			bodyDef.fixedRotation = rigidbody->fixedRotation;
			bodyDef.linearDamping = rigidbody->GetLinearDrag();
			bodyDef.angularDamping = rigidbody->GetAngularDrag();

			body = m_PhysicsWorld->CreateBody(&bodyDef);
			rigidbody->SetBody(body);
		}

		if (object->HasComponent<BoxCollider>())
		{
			BoxCollider* collider = object->GetComponent<BoxCollider>();

			float sizeX = collider->size.x * object->transform->scale.x;
			float sizeY = collider->size.y * object->transform->scale.y;
			b2Vec2 center = b2Vec2(collider->offset.x, collider->offset.y);

			b2PolygonShape boxShape;
			boxShape.SetAsBox(abs(sizeX), abs(sizeY), center, 0);

			b2FixtureDef fixtureDef;
			fixtureDef.shape = &boxShape;
			fixtureDef.density = collider->material.dencity;
			fixtureDef.restitution = collider->material.restitution;
			fixtureDef.friction = collider->material.friction;
			fixtureDef.isSensor = collider->isTrigger;

			if (body == nullptr)
				body = m_PhysicsWorld->CreateBody(&bodyDef);
			
			body->CreateFixture(&fixtureDef);
		}

		if (object->HasComponent<CircleCollider>())
		{
			CircleCollider* collider = object->GetComponent<CircleCollider>();
			b2CircleShape circleShape;
			circleShape.m_radius = collider->radius;

			b2FixtureDef fixtureDef;
			fixtureDef.shape = &circleShape;
			fixtureDef.density = collider->material.dencity;
			fixtureDef.restitution = collider->material.restitution;
			fixtureDef.friction = collider->material.friction;
			fixtureDef.isSensor = collider->isTrigger;

			if (body == nullptr)
				body = m_PhysicsWorld->CreateBody(&bodyDef);

			body->CreateFixture(&fixtureDef);
		}
	}
}

//Scene::Scene(const Scene& other)
//{
//	m_Name = other.m_Name;
//	std::cout << "Scene coping!!!\n" << m_Name << std::endl;
//
//	m_GameObjects.resize(other.m_GameObjects.size());
//	m_Gravity = other.m_Gravity;
//
//	for (size_t i = 0; i < other.m_GameObjects.size(); i++)
//	{
//		m_GameObjects[i] = std::make_unique<GameObject>(this, *other.m_GameObjects[i]);
//	}
//}

void Scene::Copy(const Scene& other)
{
	m_Name = other.m_Name;
	std::cout << "Scene coping!!!\n" << m_Name << std::endl;

	m_GameObjects.resize(other.m_GameObjects.size());
	m_Gravity = other.m_Gravity;

	for (size_t i = 0; i < other.m_GameObjects.size(); i++)
	{
		m_GameObjects[i] = std::make_unique<GameObject>(this, *other.m_GameObjects[i]);
	}
}

GameObject* Scene::CreateGameObject()
{
	WARN("Game Object created");

	m_GameObjects.push_back(std::make_unique<GameObject>(this));
	return m_GameObjects.back().get();
}

GameObject* Scene::GetObjectWithID(int ID)
{
	for (auto& object : m_GameObjects)
	{
		if (object->GetID() == ID)
			return object.get();
	}

	return nullptr;
}

void Scene::UpdateOnEditor(EditorCamera& camera)
{
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, 1920, 1080);

	//for (auto& renderObj : m_RenderObjects)
	//{
	//	if (renderObj != nullptr)
	//	{
	//		/*if (renderObj->gameObject->IsActive() && renderObj->IsEnabled())
	//		{*/
	//		Shader& shader = renderObj->GetShader();
	//		shader.Bind();
	//		shader.SetMat3("camera", camera.GetCameraMatrix());

	//		renderObj->Draw();
	//		//}
	//	}
	//}

	auto renderObjects = GetObjectsComponent<SpriteRenderer>();
	for (auto& renderObj : renderObjects)
	{
		if (renderObj != nullptr)
		{
			Shader& shader = renderObj->GetShader();
			shader.Bind();
			shader.SetMat3("camera", camera.GetCameraMatrix());
			renderObj->Draw();
		}
	}
}

void Scene::Start()
{
	m_PhysicsWorld = std::make_unique<b2World>(m_Gravity);
	m_PhysicsWorld->SetContactListener(&m_ContactListener);
	PhysicsWorldStart();

	for (auto& object : m_GameObjects)
		object->ComponentsStart();

	/*for (auto& object : m_GameObjects)
{
	if (object->HasComponent<Rigidbody>())
	{
		Rigidbody* rigidbody = object->GetComponent<Rigidbody>();

		b2FixtureDef fixtureDef;
		fixtureDef.density = 1.0f;
		fixtureDef.friction = rigidbody->material.friction;
		fixtureDef.restitution = rigidbody->material.restitution;

		b2PolygonShape boxShape;
		if (object->HasComponent<BoxCollider>())
		{
			BoxCollider* collider = object->GetComponent<BoxCollider>();

			float hx = collider->size.x * object->transform->scale.x;
			float hy = collider->size.y * object->transform->scale.y;
			b2Vec2 center = b2Vec2(collider->offset.x, collider->offset.y);
			boxShape.SetAsBox(abs(hx), abs(hy), center, 0);

			fixtureDef.shape = &boxShape;
		}

		b2CircleShape circleShape;
		if (object->HasComponent<CircleCollider>())
		{
			CircleCollider* collider = object->GetComponent<CircleCollider>();

			circleShape.m_radius = collider->radius;
			fixtureDef.shape = &circleShape;
		}

		b2BodyDef bodyDef;
		bodyDef.type = b2_dynamicBody;

		Vector2f pos = object->transform->position;
		float angle = ToRads(-object->transform->angle);

		bodyDef.position = b2Vec2(pos.x, pos.y);
		bodyDef.angle = angle;
		bodyDef.fixedRotation = false;

		b2Body* body = m_PhysicsWorld->CreateBody(&bodyDef);
		body->CreateFixture(&fixtureDef);

		rigidbody->body = body;
	}
	else if (object->HasComponent<BoxCollider>())
	{
		BoxCollider* collider = object->GetComponent<BoxCollider>();

		b2BodyDef bodyDef;
		Vector2f pos = object->transform->position;
		float angle = ToRads(-object->transform->angle);

		bodyDef.position = b2Vec2(pos.x, pos.y);
		bodyDef.angle = angle;

		b2Body* body = m_PhysicsWorld->CreateBody(&bodyDef);
		float sizeX = collider->size.x * object->transform->scale.x;
		float sizeY = collider->size.y * object->transform->scale.y;
		b2Vec2 center = b2Vec2(collider->offset.x, collider->offset.y);
		b2PolygonShape box;
		box.SetAsBox(abs(sizeX), abs(sizeY), center, 0);

		body->CreateFixture(&box, 0.0f);
	}
	else if (object->HasComponent<CircleCollider>())
	{
		/*CircleCollider* collider = object->GetComponent<CircleCollider>();

		b2BodyDef bodyDef;
		Vector2f pos = object->transform->position;
		bodyDef.position.Set(pos.x, pos.y);

		b2Body* circle = m_PhysicsWorld->CreateBody(&bodyDef);
		b2CircleShape circleBody;
		circleBody.m_radius = collider->radius;

		circle->CreateFixture(&circle, 0.0f);
	}
}*/
}

void Scene::Stop()
{
	m_PhysicsWorld.reset();
}

void Scene::Update()
{
	begin = std::chrono::steady_clock::now();
	int64_t duration = std::chrono::duration_cast<std::chrono::milliseconds>(begin - end).count();

	//Fixed Update
	if (duration >= 15)
	{
		end = std::chrono::steady_clock::now();

		int32 velocityIterations = 6;
		int32 positionIterations = 2;
		m_PhysicsWorld->Step(0.015f, velocityIterations, positionIterations);

		for (auto& object : m_GameObjects)
		{
			if (object->IsActive())
				object->ComponentsFixedUpdate();
		}
	}

	//Update
	for (auto& object : m_GameObjects)
	{
		if (object->IsActive())
			object->ComponentsUpdate();
	}

	//Late Update
	for (auto& object : m_GameObjects)
	{
		if (object->IsActive())
			object->ComponentsLateUpdate();
	}
}

void Scene::Resize(unsigned int width, unsigned int heigth)
{
	for (auto& object : m_GameObjects)
	{
		if (object->HasComponent<Camera>())
		{
			object->GetComponent<Camera>()->SetRatio(float(heigth) / float(width));
		}
	}
}

void Scene::Render()
{
	Camera* mainCamera = nullptr;
	for (auto& object : m_GameObjects)
	{
		if (object->HasComponent<Camera>())
		{
			mainCamera = object->GetComponent<Camera>();
		}
	}

	if (mainCamera != nullptr)
	{
		Color color = mainCamera->backgroundColor;
		glClearColor(color.r, color.g, color.b, color.a);
		glClear(GL_COLOR_BUFFER_BIT);

		//for (auto& it : m_RenderObjects)
		//{
		//	if (it != nullptr)
		//	{
		//		/*if (it->gameObject->IsActive() && it->IsEnabled())
		//		{*/
		//		Shader& shader = it->GetShader();
		//		shader.Bind();
		//		shader.SetMat3("camera", mainCamera->GetCameraMatrix());

		//		it->Draw();
		//		//}
		//	}
		//}

		auto renderObjects = GetObjectsComponent<SpriteRenderer>();
		for (auto& renderObj : renderObjects)
		{
			if (renderObj != nullptr)
			{
				Shader& shader = renderObj->GetShader();
				shader.Bind();
				shader.SetMat3("camera", mainCamera->GetCameraMatrix());
				renderObj->Draw();
			}
		}
	}
}

Scene::~Scene()
{
	std::cout << "Scene Destructor!!!\n";
}