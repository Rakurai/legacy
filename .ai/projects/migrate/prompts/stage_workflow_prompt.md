# AI Agent Stage Workflow & Iteration Prompt

## Purpose
This prompt guides the AI agent in executing the workflow and iteration process for a project stage. The agent should use available tools (including git, code analysis, and documentation utilities) to ensure high-quality, well-documented, and reviewable results. Robust documentation and traceability are essential, as the agentic process may be interrupted and resumed by a different agent or user at any time.

---

## Stage Workflow Instructions

1. **Review File Summaries and Context**
   - Review `.ai/docs/files_old.md`, `.ai/docs/files_new.md`, and `.ai/docs/files_docs.md` for summaries of all original C++ source files, new Python files, and documentation files. These serve as a directory and context reference.
   - If new or removed files are discovered, update these summaries accordingly.

2. **Review Stage Outline and Prior Documentation**
   - Read the current stage outline in `.ai/projects/migrate/plan.md` and review documentation from all previous stages in `.ai/projects/migrate/stages/` to ensure alignment with the overall vision and project history.
   - If discrepancies or ambiguities are found that cannot be resolved, pause and clarify with the user.

3. **Review Relevant Code & Documentation**
   - Analyze the relevant codebase sections and associated documentation.
   - Identify dependencies, affected modules, and any legacy considerations. Note these in the summary docs in `.ai/docs/`.

4. **Clarify & Expand the Plan**
   - Conduct a Q/A session (with the human developer or via self-query) to clarify ambiguities and expand the plan into a detailed, actionable checklist.
   - If important design issues are discovered, present them to the user with suggested solutions, and document the user's decision and reasoning in the revised action plan.
   - Document any assumptions, open questions, or risks.

5. **Update Stage Documentation**
   - Update or create documentation for this stage in `.ai/projects/migrate/stages/` (e.g., `stage_X_<name>.md`) to reflect the detailed action plan, clarifications, and any design decisions.
   - Ensure documentation is clear, concise, and up-to-date. Log all significant changes in `.ai/projects/migrate/change_log.md`.
   - Record session metadata (date, agent version, summary of actions) in `.ai/projects/migrate/session_log.md` at the start and end of the stage.
   - Record any user preferences or decisions in `.ai/projects/migrate/user_preferences.md`.

6. **Execute the Plan**
   - Implement the planned changes using best practices and project standards.
   - Use git for version control: a single commit to the development branch is sufficient for each stage in this phase.
   - Use available tools for code quality (linting, formatting, testing, etc.).

7. **Pause for Clarification if Needed**
   - If any issues, blockers, or uncertainties arise, pause execution and request clarification from the human developer.
   - Document the issue and any attempted resolutions in the current stage file in `.ai/projects/migrate/stages/`.

8. **Describe Reasoning & Changes**
   - For each significant change, document the reasoning, alternatives considered, and why the chosen approach was selected in the current stage file.
   - Summarize the impact of changes on the codebase and project goals.
   - Update `.ai/projects/migrate/change_log.md` and relevant summary docs in `.ai/docs/`.

9. **Iterate Until Satisfactory**
   - Repeat steps 4–8 as needed, incorporating feedback and refining the implementation until the stage goals are met.

10. **Commit & Finalize**
    - Finalize the commit, ensure all documentation is updated, and prepare a summary report of the stage in `.ai/projects/migrate/stages/`.
    - Archive the action plan, change report, and checklist in the current stage file.
    - Leave a "next steps" or "pending questions" section in the stage documentation to aid future agents or users.

---

## General Guidance
- Prioritize clarity, maintainability, and alignment with project goals.
- Communicate proactively and document all decisions and changes.
- Use automation and available tools to ensure code quality and consistency.
- Always pause and seek clarification if unsure about requirements or implementation details.
- Maintain robust documentation and traceability to support seamless handoff between agent sessions.

---

This prompt should be referenced at the start of each project stage to guide the AI agent’s workflow and ensure a consistent, high-quality, and traceable process.
