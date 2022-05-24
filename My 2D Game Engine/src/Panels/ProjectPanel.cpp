#include "ProjectPanel.h"
#include "imgui.h"


ProjectPanel::ProjectPanel()
	:m_CurrentDirectory("Assets")
	,m_BackArrowIcon("Resources/BackArrow.png")
	,m_FolderIcon("Resources/FolderIcon.png")
	,m_FileIcon("Resources/FileIcon.png")

{

}

void ProjectPanel::ImGuiRender()
{
	//ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("Project ");

	ImGui::PushStyleColor(ImGuiCol_Button, {});
	ImGui::ImageButton((ImTextureID)m_BackArrowIcon.GetID(), { 30, 30 });
	ImGui::PopStyleColor();

	for (auto& directoryEntity : std::filesystem::path("Assets"))
	{
		if (ImGui::IsItemClicked() && m_CurrentDirectory != "Assets")
		{
			m_CurrentDirectory = m_CurrentDirectory.parent_path();
		}
	}
	
	float padding = 15;
	float iconSize = 90;
	float cellSize = iconSize + padding;

	float panelWidth = ImGui::GetContentRegionAvail().x;
	int columnCount = (int)(panelWidth / cellSize);
	if (columnCount < 1)
		columnCount = 1;
	
	ImGui::Columns(columnCount, 0, false);

	for (auto& directory : std::filesystem::directory_iterator(m_CurrentDirectory))
	{
		const auto& path = directory.path();
		auto relativePath = std::filesystem::relative(path, "Assets");
		std::string filenameString = relativePath.filename().string();

		ImGui::PushID(filenameString.c_str());
		Texture& icon = directory.is_directory() ? m_FolderIcon : m_FileIcon;
		ImGui::PushStyleColor(ImGuiCol_Button, { 0, 0, 0, 0 });
		ImGui::ImageButton((ImTextureID)icon.GetID(), { iconSize, iconSize }, { 0, 1 }, {1, 0});
		ImGui::PopStyleColor();

		if (ImGui::BeginDragDropSource())
		{
			const wchar_t* itemPath = relativePath.c_str();
			ImGui::SetDragDropPayload("PROJECT_PANEL_ITEM", itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t));
			ImGui::EndDragDropSource();
		}

		if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
		{
			if (directory.is_directory())
				m_CurrentDirectory /= path.filename();
		}
		ImGui::TextWrapped(filenameString.c_str());

		ImGui::NextColumn();

		ImGui::PopID();
	}
	ImGui::Columns(1);

	ImGui::End();
}