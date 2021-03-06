#include "Inspector.h"
#include "src/ScriptManager.h"

using namespace std::string_literals;

Inspector::Inspector(Ptr<GameObject>& gameObjectRef)
	:m_SelectedGameObject(gameObjectRef)
{
}

void Inspector::ImGuiRender()
{
	ImGui::Begin("Inspector");

	if (m_SelectedGameObject != nullptr)
	{
		DrawGameObjectProperties();
		RenderComponents();
		ImGuiAddComponentButton();
	}

	ImGui::End();
}

void Inspector::DrawGameObjectProperties()
{
	bool active = m_SelectedGameObject->IsActive();
	ImGui::Checkbox("##IsActive", &active);
	m_SelectedGameObject->SetActive(active);

	ImGui::SameLine();

	ImGui::InputText("##", m_SelectedGameObject->m_Name, 20);

	ImGui::Columns(2, "##Tags and Layers", false);
	ImGui::SetColumnWidth(0, ImGui::GetWindowSize().x / 2.0f);

	ImGui::Text("Tag");
	ImGui::SameLine();
	if (ImGui::BeginCombo("##Tags", "Untagged"))
	{
		ImGui::EndCombo();
	}

	ImGui::NextColumn();
	ImGui::SetColumnWidth(1, ImGui::GetWindowSize().x / 2.0f);

	ImGui::Text("Layer");
	ImGui::SameLine();
	if (ImGui::BeginCombo("##Layers", "Default"))
	{
		ImGui::EndCombo();
	}

	ImGui::Columns(1);

	ImGui::Separator();
}

void Inspector::RenderComponents()
{
	m_SecondCollumnWidth = ImGui::GetWindowSize().x - m_FirstCollumnWidth;
	m_ItemWidth = m_SecondCollumnWidth - 15;

	auto& components = m_SelectedGameObject->GetComponents();
	for (auto& it : components)
		RenderComponents<AllComponents>(it.get());

	for (auto& script : m_SelectedGameObject->GetScripts())
	{
		ImGui::CollapsingHeader(script->GetName().c_str());
		ImGui::Separator();
	}

	ImGui::NewLine();
	ImGui::NewLine();
}

bool Inspector::RenderComponentHeader(const std::string& componentName, Component* component, bool isEditable)
{
	bool isOpen = ImGui::CollapsingHeader(("##"s + componentName).c_str(), ImGuiTreeNodeFlags_AllowItemOverlap);
	bool deleteComponent = false;
	ImGui::SameLine();

	ImGui::PushID(componentName.c_str());
	if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
		ImGui::OpenPopup("Component properties");

	if (ImGui::BeginPopup("Component properties"))
	{
		if (ImGui::MenuItem("Delete"))
			deleteComponent = true;

		ImGui::EndPopup();
	}
	ImGui::PopID();

	if (isEditable)
	{
		bool isEnabled = component->IsEnabled();
		ImGui::Checkbox(("##Is Enabled"s + componentName).c_str(), &isEnabled);
		component->SetEnabled(isEnabled);
	}

	ImGui::SameLine();
	ImGui::Text(componentName.c_str());

	if (deleteComponent)
	{
		m_SelectedGameObject->RemoveComponent(component);
		return false;
	}

	return isOpen;
}

void Inspector::RenderComponent(Transform* transform)
{
	if (ImGui::CollapsingHeader("Transform"))
	{
		ImGui::Columns(2, 0, false);
		ImGui::SetColumnWidth(0, m_FirstCollumnWidth);

		ImGui::Spacing();
		ImGui::Text("Position");
		ImGui::Spacing();
		ImGui::Text("Rotation");
		ImGui::Spacing();
		ImGui::Text("Scale");

		ImGui::NextColumn();
		ImGui::SetColumnWidth(1, m_SecondCollumnWidth);

		ImGui::SetNextItemWidth(m_ItemWidth);
		ImGui::DragFloat2("##Position ", (float*)&transform->position, 0.02f);
		ImGui::SetNextItemWidth(m_ItemWidth);
		ImGui::DragFloat("##Rotation", &transform->angle, 0.02f);
		ImGui::SetNextItemWidth(m_ItemWidth);
		ImGui::DragFloat2("##Scale  ", (float*)&transform->scale.x, 0.01f);

		ImGui::Columns(1);
	}
}

void Inspector::RenderComponent(SpriteRenderer* spriteRenderer)
{
	bool isOpen = RenderComponentHeader("SpriteRenderer", spriteRenderer, true);

	if (isOpen)
	{
		ImGui::Columns(2, 0, false);
		ImGui::SetColumnWidth(0, m_FirstCollumnWidth);

		ImGui::Spacing();
		ImGui::Text("Order In Layer");
		ImGui::Spacing();
		ImGui::Text("Color");
		ImGui::Spacing();
		ImGui::Text("Flip X");
		ImGui::Spacing();
		ImGui::Text("Flip Y");
		ImGui::Spacing();
		ImGui::Text("Texture");

		ImGui::NextColumn();
		ImGui::SetColumnWidth(1, m_SecondCollumnWidth);

		ImGui::SetNextItemWidth(m_ItemWidth);
		ImGui::DragInt("##Order In Layer", &spriteRenderer->orderInLayer, 1, 0);

		ImGui::SetNextItemWidth(m_ItemWidth);
		if (ImGui::ColorEdit4("##Color", (float*)&spriteRenderer->m_Color))
		{
			Color color = spriteRenderer->m_Color;
			spriteRenderer->m_Shader.Bind();
			spriteRenderer->m_Shader.SetVec4f("spriteColor", color.r, color.g, color.b, color.a);
		}

		bool flipX = spriteRenderer->GetFlipX();
		if (ImGui::Checkbox("##FlipX", &flipX))
			spriteRenderer->SetFlipX(flipX);

		bool flipY = spriteRenderer->GetFlipY();
		if (ImGui::Checkbox("##FlipY", &flipY))
			spriteRenderer->SetFlipY(flipY);

		ImGui::ImageButton((ImTextureID)spriteRenderer->m_Sprite.GetID(), { float(m_ItemWidth), float(m_ItemWidth) }, { 0, 1 }, { 1, 0 });
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("PROJECT_PANEL_ITEM"))
			{
				const wchar_t* path = (const wchar_t*)payload->Data;
				std::filesystem::path texturePath = "Assets";
				texturePath /= path;

				std::wstring wPath = texturePath.c_str();
				std::string sPath(wPath.begin(), wPath.end());
				spriteRenderer->m_Sprite = Texture(sPath);
				//m_Sprite = Texture(path);
				//ImGui::GetDragDropPayload();
				ImGui::EndDragDropTarget();
			}
		}

		ImGui::Columns(1);
	}
}

void Inspector::RenderComponent(Camera* camera)
{
	bool isOpen = RenderComponentHeader("Camera", camera, true);

	if (isOpen)
	{
		ImGui::Columns(2, 0, false);
		ImGui::SetColumnWidth(0, m_FirstCollumnWidth);

		ImGui::Spacing();
		ImGui::Text("Background");
		ImGui::Spacing();
		ImGui::Text("Size"); ImGui::SameLine();

		ImGui::NextColumn();
		ImGui::SetColumnWidth(1, m_SecondCollumnWidth);

		ImGui::SetNextItemWidth(m_ItemWidth);
		ImGui::ColorEdit4("##Background Color", (float*)&camera->backgroundColor);
		ImGui::SetNextItemWidth(m_ItemWidth);
		ImGui::DragFloat("##Size", &camera->size, 0.001f);

		ImGui::Columns(1);
	}
}

void Inspector::RenderComponent(Rigidbody* rigidbody)
{
	bool isOpen = RenderComponentHeader("Rigidbody", rigidbody, false);

	if (isOpen)
	{
		ImGui::Columns(2, 0, false);
		ImGui::SetColumnWidth(0, m_FirstCollumnWidth);

		ImGui::Spacing();
		ImGui::Text("Body Type");
		ImGui::Spacing();
		ImGui::Text("Mass");
		ImGui::Spacing();
		ImGui::Text("Gravity Scale");
		ImGui::Spacing();
		ImGui::Text("Linear  Drag");
		ImGui::Spacing();
		ImGui::Text("Angular Drag");

		ImGui::NewLine();
		ImGui::Spacing();

		ImGui::Text("Fixed Rotation");

		ImGui::NextColumn();
		ImGui::SetColumnWidth(1, m_SecondCollumnWidth);

		std::string bodyTypeStr;
		if (rigidbody->type == BodyType(b2_dynamicBody))
			bodyTypeStr = "Dynamic";
		else if (rigidbody->type == BodyType(b2_staticBody))
			bodyTypeStr = "Static";
		else
			bodyTypeStr = "Kinematic";

		ImGui::SetNextItemWidth(m_ItemWidth);
		if (ImGui::BeginCombo("##BodyType", bodyTypeStr.c_str()))
		{
			if (ImGui::Selectable("Dynamic"))
				rigidbody->type = BodyType(b2_dynamicBody);
			if (ImGui::Selectable("Static"))
				rigidbody->type = BodyType(b2_staticBody);
			if (ImGui::Selectable("Kinematic"))
				rigidbody->type = BodyType(b2_kinematicBody);

			ImGui::EndCombo();
		}

		ImGui::SetNextItemWidth(m_ItemWidth);
		ImGui::DragFloat("##Mass", &rigidbody->m_Mass, 0.01f);

		ImGui::SetNextItemWidth(m_ItemWidth);
		ImGui::DragFloat("##GravityScale", &rigidbody->gravityScale, 0.01f);

		ImGui::SetNextItemWidth(m_ItemWidth);
		ImGui::DragFloat("##LinearDrag", &rigidbody->m_LinearDrag, 0.01f);

		ImGui::SetNextItemWidth(m_ItemWidth);
		ImGui::DragFloat("##AngularDrag", &rigidbody->m_AngularDrag, 0.01f);

		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Checkbox("##FixedRotation", &rigidbody->fixedRotation);
		ImGui::Columns(1);
	}
}

void Inspector::RenderComponent(BoxCollider* boxCollider)
{
	bool isOpen = RenderComponentHeader("BoxCollider", boxCollider, true);

	if (isOpen)
	{
		ImGui::Columns(2, 0, false);
		ImGui::SetColumnWidth(0, m_FirstCollumnWidth);

		ImGui::Spacing();
		ImGui::Text("Size");
		ImGui::Spacing();
		ImGui::Text("Offset");
		ImGui::Spacing();
		ImGui::Text("Is Trigger");
		ImGui::Spacing();
		ImGui::Text("Material");
		ImGui::Spacing();
		ImGui::Text("Friction");
		ImGui::Spacing();

		ImGui::NextColumn();
		ImGui::SetColumnWidth(1, m_SecondCollumnWidth);

		ImGui::SetNextItemWidth(m_ItemWidth);
		ImGui::DragFloat2("##Size", (float*)&boxCollider->size, 0.01f, 0.0f, FLT_MAX, "%.3f");
		ImGui::SetNextItemWidth(m_ItemWidth);
		ImGui::DragFloat2("##Offset", (float*)&boxCollider->offset, 0.1f);
		ImGui::Checkbox("##IsTrigger", &boxCollider->isTrigger);
		ImGui::NewLine();
		ImGui::SetNextItemWidth(m_ItemWidth);
		ImGui::DragFloat("##Friction", &boxCollider->material.friction, 0.01f, 0.0f, FLT_MAX);
		//ImGui::Text("Bounciness     ");
		//ImGui::SameLine();
		//ImGui::DragFloat("##Bounciness", &material.restitution, 0.01f, 0.0f, FLT_MAX);
		ImGui::Columns(1);
	}
}

void Inspector::RenderComponent(CircleCollider* circleCollider)
{
	bool isOpen = RenderComponentHeader("CircleCollider", circleCollider, true);

	if (isOpen)
	{
		ImGui::Columns(2, 0, false);
		ImGui::SetColumnWidth(0, m_FirstCollumnWidth);

		ImGui::Spacing();
		ImGui::Text("Radius");
		ImGui::Spacing();

		ImGui::NextColumn();
		ImGui::SetColumnWidth(1, m_SecondCollumnWidth);

		ImGui::SetNextItemWidth(m_ItemWidth);
		ImGui::DragFloat("##Radius", &circleCollider->radius, 0.001f, 0.0f);

		ImGui::Columns(1);
	}
}

void Inspector::RenderComponent(PolygonCollider* polygonCollider)
{
	bool isOpen = RenderComponentHeader("PolygonCollider", polygonCollider, true);

	if (isOpen)
	{

	}
}

void Inspector::RenderComponent(Animator* animator)
{
	bool isOpen = RenderComponentHeader("Animator", animator, true);

	if (isOpen)
	{

	}
}

void Inspector::ImGuiAddComponentButton()
{
	int center = ImGui::GetWindowSize().x / 2;
	ImGui::SameLine(center - 100);
	if (ImGui::Button("Add Component", ImVec2(200, 20)))
	{
		ImGui::OpenPopup("Components");
	}

	if (ImGui::BeginPopup("Components"))
	{
		if (ImGui::MenuItem("Sprite Renderer"))
			m_SelectedGameObject->AddComponent<SpriteRenderer>();

		if (ImGui::MenuItem("Camera"))
			m_SelectedGameObject->AddComponent<Camera>();

		if (ImGui::MenuItem("Rigidbody"))
			m_SelectedGameObject->AddComponent<Rigidbody>();

		if (ImGui::MenuItem("Box Collider"))
			m_SelectedGameObject->AddComponent<BoxCollider>();

		if (ImGui::MenuItem("Circle Collider"))
			m_SelectedGameObject->AddComponent<CircleCollider>();

		if (ImGui::MenuItem("Polygon Collider"))
			m_SelectedGameObject->AddComponent<PolygonCollider>();

		if (ImGui::MenuItem("Animator"))
			m_SelectedGameObject->AddComponent<Animator>();

		ImGui::Separator();

		for (auto& it : ScriptManager::Get().GetScripts())
		{
			if (ImGui::MenuItem(it.first.c_str()))
			{
				Script* script = it.second->Create();
				script->SetName(it.first);
				m_SelectedGameObject->AddScript(script);
			}
		}

		ImGui::EndPopup();
	}
}