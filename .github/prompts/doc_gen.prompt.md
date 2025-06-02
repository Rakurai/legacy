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

2. **Discovery Phase**
   - Gather the list of header and source files from `.ai/context/files_old.md` to reference the file paths and the number of lines of code for each file.
   - Files that already have summaries have been processed by a previous agent and can be skipped.

3. **Batching and Processing of Header Files**
   - For all batching and summarization of header (`.h`, `.hh`) files, follow the detailed instructions in `.github/prompts/doc_gen_batch.prompt.md`. This batch prompt governs grouping, processing, and documentation of header files.
   - Iterate on the instructions until all header files in `.ai/context/files_old.md` have been summarized.

4. **Batching and Processing of Implementation Files**
   - For all batching and summarization of source (`.c`, `.cpp`, `.cc`) files, follow the detailed instructions in `.github/prompts/doc_gen_batch.prompt.md`. This batch prompt governs grouping, processing, and documentation of implementation files.
   - Iterate on the instructions until all implementation files in `.ai/context/files_old.md` have been summarized.

5. **Combine & Integrate Results**
   - Integrate header and implementation summaries to produce module-level and system-level overviews.
   - Update `.ai/docs/component_summaries.md`, `.ai/docs/architecture_overview.md`, and `.ai/docs/system_overview.md` as needed.

---

## General Guidance
- Prioritize clarity, completeness, and maintainability in all documentation.
- Communicate proactively and document all decisions and changes.
- Use automation and available tools to ensure documentation quality and consistency.
- Always pause and seek clarification if unsure about requirements or documentation details.
- Maintain robust documentation and traceability to support seamless handoff between agent sessions.

---

This prompt should be referenced at the start of each documentation generation stage to guide the AI agent’s workflow and ensure a consistent, high-quality, and traceable process.
