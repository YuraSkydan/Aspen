#include "GameObject.h"
#include "../ScriptManager.h"
#include "../Components/BoxCollider.h"
#include "../Components/CircleCollider.h"
#include "../Components/Rigidbody.h"

GameObject::GameObject(Scene* scene)
	:m_Scene(scene)
{
	auto newComponent = std::make_unique<Transform>(this);
	transform = newComponent.get();
	m_Components.push_back(std::move(newComponent));

	std::stringstream name;
	name << "GameObject" << s_Objects << '\0';
	name >> m_Name;

	m_ID = s_Objects;
	s_Objects++;
}

GameObject::GameObject(Scene* scene, const GameObject& other)
	:m_Scene(scene)
{
	auto newComponent = std::make_unique<Transform>(this);
	transform = newComponent.get();
	m_Components.push_back(std::move(newComponent));

	strcpy_s(m_Name, 20, other.m_Name);
	m_IsActive = other.m_IsActive;
	m_ID = other.m_ID;

	CopyComponents<AllComponents>(other);

	for (auto& it : other.m_Scripts)
	{
		auto& scripts = ScriptManager::GetInstance().GetScripts();
		Script* newScript = scripts.find(it->GetName())->second->Create();
		
		newScript->SetName(it->GetName());
		newScript->gameObject = this;
		newScript->transform = transform;

		m_Components.push_back(std::unique_ptr<Component>(newScript));
		m_Scripts.push_back(newScript);
	}
}

//maybe change copy constructor later
//GameObject::GameObject(const GameObject& other)
//{
//	auto newComponent = std::make_unique<Transform>(this);
//	transform = newComponent.get();
//	m_Components.push_back(std::move(newComponent));
//
//	strcpy_s(m_Name, 20, other.m_Name);
//
//	m_Scene = other.m_Scene;
//	m_IsActive = other.m_IsActive;
//	m_ID = other.m_ID;
//
//	CopyComponents<AllComponents>(other);
//
//	//for (auto& script : other.m_Scripts)
//	//{
//	//	script->Update();
//		//m_Scripts.push_back(script->Create());
//	//}
//}

void GameObject::SetName(const char* newName)
{
	strcpy_s(m_Name, 20, newName);
}

void GameObject::SetActive(bool active)
{
	if (m_IsActive == true && active == false)
	{
		m_IsActive = active;
		ComponentsDisable();
	}
	else if (m_IsActive == false && active == true)
	{
		m_IsActive = active;
		ComponentsEnable();
	}
}

void GameObject::ComponentsAwake()
{
	for (auto& it : m_Components)
	{
		if (it->IsEnabled())
			it->Awake();
	}
}

void GameObject::ComponentsStart()
{
	for (auto& it : m_Components)
	{
		if (it->IsEnabled())
			it->Start();
	}
}

void GameObject::ComponentsUpdate()
{
	for (auto& it : m_Components)
	{
		if (it->IsEnabled())
			it->Update();
	}
}

void GameObject::ComponentsFixedUpdate()
{
	for (auto& it : m_Components)
	{
		if (it->IsEnabled())
			it->FixedUpdate();
	}
}

void GameObject::ComponentsLateUpdate()
{
	for (auto& it : m_Components)
	{
		if (it->IsEnabled())
			it->LateUpdate();
	}
}

void GameObject::ComponentsEnable()
{
	for (auto& it : m_Components)
	{
		if (it->IsEnabled())
			it->OnEnabled();
	}
}

void GameObject::ComponentsDisable()
{
	for (auto& it : m_Components)
	{
		if (it->IsEnabled())
			it->OnDisabled();
	}
}

void GameObject::AddScript(Script* script)
{
	script->gameObject = this;
	script->transform = transform;

	m_Components.push_back(std::unique_ptr<Component>(script));
	m_Scripts.push_back(script);
}

GameObject::~GameObject()
{
	std::cout << "GameObject destructor!!!\n";
}
