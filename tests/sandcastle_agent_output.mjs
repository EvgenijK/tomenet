import assert from "node:assert/strict";
import test from "node:test";
import { parseAgentOutput, agentFailureDetail } from "../.sandcastle/agent-output.mjs";

test("JSON stdout startup errors are retained even when stderr is empty", () => {
  const captured = { message: "", tokens: 0 };
  parseAgentOutput(JSON.stringify({ type: "error", message: "The requested model gpt-6.1-sol is not supported" }), captured);
  assert.equal(agentFailureDetail(captured, ""), "The requested model gpt-6.1-sol is not supported");
  parseAgentOutput(JSON.stringify({ type: "turn.failed", error: { message: "authentication failed" } }), captured);
  assert.equal(agentFailureDetail(captured, ""), "authentication failed");
});

test("failure diagnostics redact credentials and cap output", () => {
  const captured = { message: "", tokens: 0 };
  parseAgentOutput(JSON.stringify({ type: "turn.failed", error: { message: 'Authorization: Bearer abc.secret, api_key="private-key" access_token=token-value password=pwd https://user:pass@example.com/x' } }), captured);
  const detail = agentFailureDetail(captured, "");
  for (const secret of ["abc.secret", "private-key", "token-value", "pwd", "user:pass"]) assert.ok(!detail.includes(secret));
  assert.match(detail, /REDACTED/);
  assert.ok(agentFailureDetail({}, "x".repeat(4000)).length <= 2000);
});

test("messages and usage retain their existing semantics", () => {
  const captured = { message: "", tokens: 0 };
  parseAgentOutput("not JSON", captured, () => {});
  parseAgentOutput(JSON.stringify({ type: "item.completed", item: { type: "agent_message", text: "result" } }), captured, () => {});
  parseAgentOutput(JSON.stringify({ type: "turn.completed", usage: { input_tokens: 7, output_tokens: 3 } }), captured, () => {});
  assert.equal(captured.message, "result");
  assert.equal(captured.tokens, 10);
  assert.match(agentFailureDetail({}, ""), /no diagnostic/);
});
