# AI Agent Documentation Generation Workflow & Iteration Prompt

---
mode: 'agent'
tools: ['githubRepo', 'codebase']
description: 'Generate codebase documentation'
---
Your goal is to execute the workflow and iteration process for a documentation generation stage. You should use available tools (including code analysis, file discovery, and documentation utilities) to ensure comprehensive, well-organized, and reviewable documentation. Robust documentation and traceability are essential, as the generation process may be interrupted and resumed by a different agent or user at any time.

---

## Stage Workflow Instructions

1. **Review File Summaries and Context**
   - Review `.ai/context/files_old.md` for summaries of all original (legacy) source files, `.ai/context/files_docs.md` for documentation files, and `.ai/context/dependency_graph.md` for file/module relationships. These serve as a directory and context reference.

2. **Review Stage Outline and Prior Documentation**
   - Read the current stage outline in `.ai/projects/doc_gen/plan.md` and review documentation from all previous stages in `.ai/projects/doc_gen/stages/` to ensure alignment with the overall vision and project history.
   - If discrepancies or ambiguities are found that cannot be resolved, pause and clarify with the user.

3. **Discovery Phase**
   - Gather the list of header and source files from `.ai/context/files_old.md` to reference the file paths and the number of lines of code for each file.

4. **Batching and Processing**
   - For header file summarization, group files by their most specific (deepest) subdirectory within `src/`.
   - Process subdirectories first, then higher-level directories if any header files remain.
   - For each batch (directory), generate a summary with key structures, functions, and dependencies, referencing `.ai/context/dependency_graph.md` for context.
   - Store batch summaries in `.ai/docs/summary_headers_batch_<subdir>.md` and update `.ai/context/files_old.md` with new insights for each file.
   - Log open questions and assumptions for each batch.

5. **Implementation File Summarization**
   - After header files, repeat the batching and summarization process for implementation files (`.c`, `.cpp`, `.cc`, etc.), using header summaries for context.
   - Follow the same directory-based batching and documentation process.

6. **Combine & Integrate Results**
   - Integrate header and implementation summaries to produce module-level and system-level overviews.
   - Update `.ai/docs/component_summaries.md`, `.ai/docs/architecture_overview.md`, and `.ai/docs/system_overview.md` as needed.

7. **Documentation & Planning**
   - Refine and expand documentation in `.ai/docs/` as specified in the action plan.
   - Ensure all new or updated documentation files are listed in `.ai/context/files_docs.md`.
   - Log all significant changes in `.ai/projects/doc_gen/change_log.md`.
   - Record session metadata in `.ai/projects/doc_gen/session_log.md` at the start and end of the stage.
   - Record any user preferences or decisions in `.ai/projects/doc_gen/user_preferences.md`.

8. **Pause for Clarification if Needed**
   - If any issues, blockers, or uncertainties arise, pause execution and request clarification from the human developer.
   - Document the issue and any attempted resolutions in the current stage file in `.ai/projects/doc_gen/stages/`.

9. **Describe Reasoning & Changes**
   - For each significant documentation change, document the reasoning, alternatives considered, and why the chosen approach was selected in the current stage file.
   - Summarize the impact of changes on the documentation corpus and project goals.
   - Update `.ai/projects/doc_gen/change_log.md` and relevant summary docs in `.ai/context/`.

10. **Iterate Until Satisfactory**
   - Repeat steps as needed, incorporating feedback and refining the documentation until the stage goals are met.

11. **Commit & Finalize**
    - Finalize all documentation updates, ensure all logs and summaries are current, and prepare a summary report of the stage in `.ai/projects/doc_gen/stages/`.
    - Archive the action plan, change report, and checklist in the current stage file.
    - Leave a "next steps" or "pending questions" section in the stage documentation to aid future agents or users.

---

## General Guidance
- Prioritize clarity, completeness, and maintainability in all documentation.
- Communicate proactively and document all decisions and changes.
- Use automation and available tools to ensure documentation quality and consistency.
- Always pause and seek clarification if unsure about requirements or documentation details.
- Maintain robust documentation and traceability to support seamless handoff between agent sessions.

---

This prompt should be referenced at the start of each documentation generation stage to guide the AI agent’s workflow and ensure a consistent, high-quality, and traceable process.
