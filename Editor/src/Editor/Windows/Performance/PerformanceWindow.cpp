#include "PerformanceWindow.h"

void PerformanceWindow::draw() {
	if (shouldClose()) return;

	if (!ImGui::Begin(("Performance###" + getWindowImGuiId()).c_str(), &isWindowOpen)) {
		ImGui::End();
		return;
	}

	if (!editor->graphsTimes.empty()) {
		ImVec2 contentSize = ImGui::GetContentRegionAvail();

		if (ImPlot::BeginPlot("FPS", ImVec2(contentSize.x / 2.0f, -1), ImPlotFlags_NoLegend | ImPlotFlags_NoMouseText | ImPlotFlags_NoFrame | ImPlotFlags_Crosshairs | ImPlotFlags_NoInputs)) {

			float max = *std::max_element(editor->graphsFPSValues.begin(), editor->graphsFPSValues.end());

			ImPlot::SetupAxesLimits(editor->graphsTimes[0], editor->graphsTimes[0] + 10, 0, max + 5, ImPlotCond_Always);
			ImPlot::SetupAxis(ImAxis_X1, nullptr, ImPlotAxisFlags_NoDecorations);

			ImPlot::PlotShaded("Data", editor->graphsTimes.data(), editor->graphsFPSValues.data(), editor->graphsTimes.size());

			ImPlot::EndPlot();
		}

		ImGui::SameLine();

		if (ImPlot::BeginPlot("RAM Usage", ImVec2(-1, -1), ImPlotFlags_NoLegend | ImPlotFlags_NoMouseText | ImPlotFlags_NoFrame | ImPlotFlags_Crosshairs | ImPlotFlags_NoInputs)) {

			float max = *std::max_element(editor->graphsMemoryValues.begin(), editor->graphsMemoryValues.end());

			ImPlot::SetupAxesLimits(editor->graphsTimes[0], editor->graphsTimes[0] + 10, 0, max + 5, ImPlotCond_Always);

			ImPlot::SetupAxis(ImAxis_X1, nullptr, ImPlotAxisFlags_NoDecorations);

			ImPlot::PlotShaded("Data", editor->graphsTimes.data(), editor->graphsMemoryValues.data(), editor->graphsTimes.size());

			ImPlot::EndPlot();
		}
	}

	ImGui::End();
}
