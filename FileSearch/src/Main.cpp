
#include "Search.h"
#include "GL/glew.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "GLFW/glfw3.h"

#define FILES_PER_THREAD 20

#define WINDOW_TITLE "FileSearch"

char StartingWorkingDir[MAX_PATH];
char FileNameRegex[128];

std::vector<FileData> Files;
std::vector<std::thread> Threads;
std::map<char*, ResultVector> SearchResults;

HANDLE Console;
ProgramSettings Settings;

std::thread FindFilesThread;
std::thread SearchFilesThread;

int WindowWidth = 1280;
int WindowHeight = 720;

void glfw_err(int err, const char* desc)
{
	printf("GLFW err code: %d : %s", err, desc);
}

void glfw_resize(GLFWwindow* window, int width, int height)
{
	WindowWidth = width;
	WindowHeight = height;
}

int ImGui_SearchPathCallback(ImGuiInputTextCallbackData *data)
{
	Files.clear();
	FindFilesThread = std::thread(FindAllFiles);
	return 1;
}

int main(int argc, char* argv[])
{

	if (!glfwInit())
	{
		printf("GLFW Init Failed!");
		return -1;
	}
	glfwSetErrorCallback(glfw_err);

	const char* glsl_version = "#version 150";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only

	GLFWwindow* window = glfwCreateWindow(WindowWidth, WindowHeight, WINDOW_TITLE, nullptr, nullptr);
	if (!window)
	{
		printf("GLFW window or context creation failed");
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	glfwSetWindowSizeCallback(window, glfw_resize);
	GLenum glewErr = glewInit();
	if (GLEW_OK != glewErr)
	{
		printf("GLEW Init error: %s", glewGetErrorString(glewErr));
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	
	int FileNameNum = 0;

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		SettingsMutex.lock();
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		{
			static float f = 0.0f;
			static int counter = 0;
			ImGui::SetNextWindowSize({ (float)WindowWidth, (float)WindowHeight}, ImGuiCond_Always);
			ImGui::SetNextWindowPos({0.f, 0.f}, ImGuiCond_Always);
			ImGui::Begin("Hello, world!", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoResize);

			if (ImGui::BeginMenuBar())
			{
				if (ImGui::BeginMenu("File"))
				{
					if (ImGui::MenuItem("Open", "Ctrl+O"))
					{ /* Do stuff */ 

					}
					ImGui::EndMenu();
				}
				ImGui::EndMenuBar();
			}
			ImGui::Separator();

			ImGui::BeginChild("Search Input", { (float)(WindowWidth / 2), (float)(WindowHeight / 100 * 30) }, false, ImGuiWindowFlags_NoDecoration);

			ImGui::Text("Search path:");
			ImGui::SameLine();
			ImGui::SetNextItemWidth((float)(WindowWidth / 100 * 40));
			ImGui::InputText("##Search path", Settings.SearchDirectory, sizeof(Settings.SearchDirectory), ImGuiInputTextFlags_CallbackCompletion, ImGui_SearchPathCallback);

			ImGui::Separator();

			ImGui::Text("Search term:");
			ImGui::SameLine();
			ImGui::SetNextItemWidth((float)(WindowWidth / 100 * 40));
			ImGui::InputText("##Search term", Settings.SearchTerm, sizeof(Settings.SearchTerm), ImGuiInputTextFlags_None);

			ImGui::Separator();

			ImGui::Text("File name:");
			ImGui::SameLine();
			ImGui::SetNextItemWidth((float)(WindowWidth / 100 * 20));
			ImGui::InputText("##File name", FileNameRegex, sizeof(FileNameRegex), ImGuiInputTextFlags_None);
			ImGui::SameLine();
			if (ImGui::Button("Add"))
			{
				if (StringSize(FileNameRegex))
				{
					AddString(&Settings.FilesToInclude, FileNameRegex);
					ClearString(FileNameRegex);
				}
			}
			
			ImGui::Separator();
			FilesMutex.lock();
			ImGui::Text("Eligible files found: %d", Files.size());
			FilesMutex.unlock();

			ImGui::Separator();

			OutputMutex.lock();
			ImGui::InputInt("Max line chars", &Settings.OutputLineLength, 32);
			OutputMutex.unlock();

			if (ImGui::Button("Search!"))
			{
				FilesMutex.lock();
				int NumOfFiles = Files.size();
				FilesMutex.unlock();
				if ((StringSize(Settings.SearchTerm) > 0) && (NumOfFiles))
				{
					SearchFilesThread = std::thread(SearchFiles);
				}
			}

			ImGui::EndChild();
			
			ImGui::SameLine();

			ImGui::BeginChild("Search List", { (float)(WindowWidth / 2), (float)(WindowHeight / 100 * 30) }, false, ImGuiWindowFlags_NoDecoration);
			
			for (int i = 0; i < STR_BUF_SIZE; i++)
			{
				char* Temp = Settings.FilesToInclude.Strings[i];
				if (Temp)
				{
					if (ImGui::Button(Temp))
					{
						RemoveString(&Settings.FilesToInclude, Temp);
					}
				}
			}
			ImGui::EndChild();

			ImGui::Separator();
			
			ImGui::BeginChild("Results", { (float)WindowWidth, (float)(WindowHeight * 0.7f) });
			for (auto it = SearchResults.begin(); it != SearchResults.end(); ++it)
			{
				char* FileName = it->first;
				ResultVector& Result = it->second;
				if (ImGui::CollapsingHeader(FileName))
				{
					for (auto vectorIt = Result.begin(); vectorIt != Result.end(); vectorIt++)
					{
						SearchResult& SearchRes = *vectorIt;

						ImGui::Text("(%d):", SearchRes.LineNumber);
						ImGui::SameLine();
						ImGui::Text(SearchRes.FirstPart);
						ImGui::SameLine(0.f, 0.f);
						ImGui::TextColored({ 0.1f, 1.f, 0.1f, 1.f }, Settings.SearchTerm);
						ImGui::SameLine(0.f, 0.f);
						ImGui::Text(SearchRes.ThirdPart);
					}
				}
			}
			ImGui::EndChild();
			ImGui::End();
		}
		SettingsMutex.unlock();
		ImGui::Render();
		int display_w, display_h;
		glfwMakeContextCurrent(window);
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwMakeContextCurrent(window);
		glfwSwapBuffers(window);

	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();

	if (FindFilesThread.joinable())
	{
		FindFilesThread.join();
	}
	if (SearchFilesThread.joinable())
	{
		SearchFilesThread.join();
	}
	return 0;
}

void ConsoleSearchFiles()
{
	if (Settings.ShowInfo) printf("Searching in: %s\n", StartingWorkingDir);

	Console = GetStdHandle(STD_OUTPUT_HANDLE);

	BeginCounter();
	FindAllFiles();
	int GetFilesDuration = EndCounter();

	if (Settings.ShowTimes) printf("File gather duration: %dms\n", GetFilesDuration);
	if (Settings.ShowStats) printf("Found %zd files.\n", Files.size());

	BeginCounter();
	int NumOfFiles = (int)Files.size();

	if ((NumOfFiles > 20) && Settings.NumberOfThreads)
	{
		int NumOfFilesPerThread = NumOfFiles / Settings.NumberOfThreads;
		int Remaining = NumOfFiles % Settings.NumberOfThreads;
		FilesIndexRange* Ranges = new FilesIndexRange[Settings.NumberOfThreads];
		int StartIndex = 0;
		for (int i = 0; i < Settings.NumberOfThreads; i++)
		{
			int EndIndex = StartIndex + NumOfFilesPerThread - 1;
			Ranges[i] = { StartIndex, EndIndex };
			StartIndex = EndIndex + 1;
		}
		if (Remaining)
		{
			FilesIndexRange& last = Ranges[Settings.NumberOfThreads - 1];
			last.End += Remaining;
		}

		for (int i = 0; i < Settings.NumberOfThreads; i++)
		{
			Threads.push_back(std::thread{ SearchFilesRange, Ranges[i] });
		}
		for (int i = 0; i < Threads.size(); i++)
		{
			Threads[i].join();
		}
	}
	else
	{
		SearchFiles();
	}

	int SearchFilesDuration = EndCounter();
	if (Settings.ShowTimes) printf("Files search duration: %dms\n", SearchFilesDuration);

	if (Settings.ShowTimes) printf("Total time: %dms\n", SearchFilesDuration + GetFilesDuration);
}

void ReadProgramProperties(char* argv[], int argc)
{
	//Set default properties that will be overriden if it exists in file
	Settings.OutputLineLength = 256;
	Settings.LongFilename = false;
	Settings.ShowTimes = false;

	FILE* file = 0;
	char* exeName = GetExePath();
	char* exeDir = GetLastDirPath(exeName);
	char* fullFileName = StringConcat(exeDir, "config.fsinfo");
	fopen_s(&file, fullFileName, "rb");
	if (file)
	{
		fseek(file, 0, SEEK_END);
		int fileSize = ftell(file);
		fseek(file, 0, SEEK_SET);
		if (!fileSize) return;
		char* fileContents = new char[fileSize + 1];
		fread(fileContents, 1, fileSize, file);
		fileContents[fileSize] = 0;

		char* line = 0;
		char* remainingContents = fileContents;
		
		do
		{
			//@Leak ReadStringLine allocates the string that returns
			line = ReadStringLine(&remainingContents);
			if (StartsWith(line, "exclude"))
			{
				char* valueStr = SkipString(line, "exclude");
				valueStr++;
				Settings.FilesToExclude = BreakStringByToken(valueStr, ',');
			}
			else if (StartsWith(line, "threads"))
			{
				Settings.NumberOfThreads = GetIntValue(line, "threads");
			}
			else if (StartsWith(line, "line"))
			{
				Settings.OutputLineLength = GetIntValue(line, "line");
			}
			else if (StartsWith(line, "filename_format"))
			{
				Settings.LongFilename = GetBoolValueWithOptions(line, "filename_format", "long", "short");
			}
			else if (StartsWith(line, "show_times"))
			{
				Settings.ShowTimes = GetBoolValue(line, "show_times");
			}
			else if (StartsWith(line, "show_stats"))
			{
				Settings.ShowStats = GetBoolValue(line, "show_stats");
			}
			else if (StartsWith(line, "show_info"))
			{
				Settings.ShowInfo = GetBoolValue(line, "show_info");
			}
		}
		while (line);
	}

	for (int i = 2; i < argc; i++)
	{
		StringBuffer& buf = Settings.FilesToInclude;
		buf.Strings[buf.Size++] = argv[i];
	}
}





