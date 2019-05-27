
#include "Search.h"
#include "GL/glew.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "GLFW/glfw3.h"

#define FILES_PER_THREAD 20
#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720
#define WINDOW_TITLE "FileSearch"

char StartingWorkingDir[MAX_PATH];
std::vector<FileData> Files;
std::vector<std::thread> Threads;
HANDLE Console;
ProgramSettings Settings;

void glfw_err(int err, const char* desc)
{
	printf("GLFW err code: %d : %s", err, desc);
}

int main(int argc, char* argv[])
{
	ReadProgramProperties(argv, argc);
#ifdef _DEBUG
	strcpy(StartingWorkingDir, "E:\\workspace\\InstantWar\\AndroidUpdate4\\externals\\engine");
	SetCurrentDirectoryA(StartingWorkingDir);
#else
	if (argc < 2) return 1;
	GetCurrentDirectoryA(MAX_PATH, StartingWorkingDir);
#endif
	
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

	GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, nullptr, nullptr);
	if (!window)
	{
		printf("GLFW window or context creation failed");
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

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


	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		{
			static float f = 0.0f;
			static int counter = 0;

			ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

			ImGui::Text("This is some useful text.");
			ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
			ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

			if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
				counter++;
			ImGui::SameLine();
			ImGui::Text("counter = %d", counter);

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}

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
	if (Settings.ShowStats) printf("Found %d files.\n", Files.size());

	BeginCounter();
	int NumOfFiles = Files.size();

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

	Settings.SearchTerm = argv[1];
	for (int i = 2; i < argc; i++)
	{
		StringBuffer& buf = Settings.FilesToInclude;
		buf.Strings[buf.Size++] = argv[i];
	}
}





